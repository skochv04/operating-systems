#define _XOPEN_SOURCE 700

#include <sys/ipc.h>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include "chat_settings.h"

int minInt(int a, int b){if (a < b) return a; else return b;}

// update the variable in signal handler
volatile bool close_flag = false;

void sig_handler(int signum) {
    close_flag = true;
}

int main() {
    // PID will be used to get unique key
    // use ftok() to generate key
    key_t key = ftok("/tmp", getpid());
    if (key == -1){
        perror("ftok");
    }

    // create queue_name by concatenating data
    char queue_name[CLIENT_BUFFER_SIZE] = {0};
    sprintf(queue_name, "/chat_client_queue_%d", key);

    // fill in POSIX parameters structure (max messages = 10)
    struct mq_attr attributes = {
            .mq_flags = 0,
            .mq_msgsize = sizeof(chat_message),
            .mq_maxmsg = 10
    };

    // create client_queue (communication from server to client)
    mqd_t client_queue = mq_open(queue_name, O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR, &attributes);
    if (client_queue == -1){
        perror("client open");
    }

    // open server_queue (communication from client to server)
    // we do not use O_CREAT, because this should do server
    mqd_t server_queue = mq_open(SERVER_QUEUE_NAME, O_RDWR, S_IRUSR | S_IWUSR, NULL);
    if (server_queue == -1){
        perror("server open");
    }

    // register handler closing client to all signals
    for (int sig = 1; sig < SIGRTMAX; sig++) {
        signal(sig, sig_handler);
    }

    // initialize first message from client to server - INIT message
    chat_message message_init = {
            .type = INIT,
            .identifier = -1
    };

    // next we need to fill in message buf with queue name
    memcpy(message_init.text, queue_name, minInt(MESSAGE_MAXSIZE - 1, strlen(queue_name)));

    // try to send message to server
    if(mq_send(server_queue, (char*)&message_init, sizeof(message_init), 0) == -1){
        perror("server mq_send");
    }

    // create pipe for communicate between parent and child
    int fd[2];
    if (pipe(fd) == -1){
        perror("pipe");
    }

    // create child process for listening to messages from server
    pid_t pid = fork();
    if (pid < 0){
        perror("fork");
    } else if (pid == 0) {
        // child process

        // close read end
        close(fd[0]);
        chat_message receive_message;
        while(!close_flag) {
            // receive message from server
            if (mq_receive(client_queue, (char*)&receive_message, sizeof(receive_message), NULL) == -1)
                perror("recieve");
            switch(receive_message.type) {
                case IDENTIFY:
                    printf("Received identifier from id %d\n", receive_message.identifier);
                    write(fd[1], &receive_message.identifier, sizeof(receive_message.identifier));
                    break;
                case TEXT:
                    printf("Received text message: %s from id %d\n", receive_message.text, receive_message.identifier);
                    break;
                case FAIL:
                    printf("Failed to connect, max number of clients achieved\n");
                    close_flag = true;
                    break;
                default:
                    printf("Unpredicted message type: %u from id %d\n", receive_message.type, receive_message.identifier);
                    break;
            }
        }
        // exit from receive loop and end child`s process
        exit(0);
    } else {
        // parent process

        // close write end
        close(fd[1]);
        int identifier = -1;

        // wait and read client identifier from child process
        if(read(fd[0], &identifier, sizeof(identifier)) == -1){
            perror("read");
        }

        char input_buffer[MESSAGE_MAXSIZE]; // static buffer for message
        while(!close_flag) {
            // get message queue attributes
            mq_getattr(server_queue, &attributes);

            // check if we can send more messages to server
            if(attributes.mq_curmsgs >= attributes.mq_maxmsg) {
                printf("Please, wait. Server is busy!\n");
                continue;
            }

            // read message to send from the standard input using fgets
            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
                perror("input");
            } else {
                // Remove newline character from input (if present)
                input_buffer[strcspn(input_buffer, "\n")] = '\0';

                chat_message message_send = {
                        .type = TEXT,
                        .identifier = identifier
                };

                // Copy message from input_buffer to chat_message text field
                strncpy(message_send.text, input_buffer, MESSAGE_MAXSIZE - 1);
                message_send.text[MESSAGE_MAXSIZE - 1] = '\0'; // Ensure null-termination

                // Send message to server
                mq_send(server_queue, (char*)&message_send, sizeof(message_send), 0);
            }
        }

        // exiting from sending loop

        // if connection was made and we received identifier number
        if (identifier != -1){
            chat_message message_close = {
                    .type = CLOSE,
                    .identifier = identifier
            };

            // notify server about closing the client
            mq_send(server_queue, (char*) &message_close, sizeof(message_close), 0);
        }

        // close queues
        mq_close(server_queue);
        mq_close(client_queue);

        // unlink queues (it will delete them after closure of all processes)
        // server queue will be unlinked in server part
        mq_unlink(queue_name);
    }
}
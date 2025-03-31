#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include "chat_settings.h"

// update the variable in signal handler
volatile bool close_flag = false;

void sig_handler(int signum) {
    close_flag = true;
}

int main() {
    // fill in POSIX parameters structure (max messages = 10)
    struct mq_attr attributes = {
            .mq_flags = 0,
            .mq_msgsize = sizeof(chat_message),
            .mq_maxmsg = 10
    };

    // create server_queue (communication from client to server)
    mqd_t server_queue = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT,  S_IRUSR | S_IWUSR, &attributes);
    if(server_queue == -1) {
        perror("server open");
    }

    // register handler closing client to all signals
    for (int sig = 1; sig < SIGRTMAX; sig++) {
        signal(sig, sig_handler);
    }
    
    chat_message receive_message;

    // client queues initialized with -1 to show that they have not been used yet
    mqd_t client_queues[MAX_CLIENTS];
    mqd_t additional_queue;
    memset(client_queues, -1, sizeof(client_queues));

    printf("Server started working\n");

    while(!close_flag) {
        // receive message from client
        mq_receive(server_queue, (char*)&receive_message, sizeof(receive_message), NULL);

        switch(receive_message.type) {
            case INIT:
                // find first free id to use
                int new_id = 0;
                while(client_queues[new_id] != -1 && new_id < MAX_CLIENTS) new_id++;

                if(new_id >= MAX_CLIENTS){
                    printf("Cannot add new client, max number of clients achieved\n");

                    // open client queue on the additional place
                    additional_queue = mq_open(receive_message.text, O_RDWR, S_IRUSR | S_IWUSR, NULL);
                    if(additional_queue == -1) {
                        perror("client open");
                    }

                    // initialize close message
                    chat_message message_fail = {
                            .type = FAIL,
                            .identifier = -1
                    };

                    // send close message to the client
                    if (mq_send(additional_queue, (char*)&message_fail, sizeof (message_fail), 0) == -1){
                        perror("server mq_send");
                    }

                    printf("Send failure of connection #%d to client\n", new_id);
                    continue;
                }

                // open client queue on the free place
                client_queues[new_id] = mq_open(receive_message.text, O_RDWR, S_IRUSR | S_IWUSR, NULL);
                if(client_queues[new_id] == -1) {
                    perror("client open");
                }

                // initialize identifier message from server to client  - IDENTIFY message
                chat_message message_identify = {
                        .type = IDENTIFY,
                        .identifier = new_id
                };

                // send identifier to the client using message
                if (mq_send(client_queues[new_id], (char*)&message_identify, sizeof (message_identify), 0) == -1){
                    perror("server mq_send");
                }

                printf("Send identifier of connection #%d to client\n", new_id);

                break;
            case TEXT:
                // write the text message to all clients beside "writer"
                for (int i = 0; i < MAX_CLIENTS; i++){
                    if(i != receive_message.identifier && client_queues[i] != -1){
                        mq_send(client_queues[i], (char*)&receive_message, sizeof(receive_message), 0);
                    }
                }
                break;
            case CLOSE:
                // close client queue
                mq_close(client_queues[receive_message.identifier]);

                // mark ID as not used
                client_queues[receive_message.identifier] = -1;
                printf("Connection with client #%d closed\n", receive_message.identifier);
                break;
            default:
                printf("Unexpected message type given\n");
                break;
        }
    }

    printf("Server stopped\n");

    // close all opened client queues
    for (int i = 0; i < MAX_CLIENTS; i++){
        if (client_queues[i] != -1) mq_close(client_queues[i]);
    }

    // close and unlink server queue
    // client queue will be unlinked in client part
    mq_close(server_queue);
    mq_unlink(SERVER_QUEUE_NAME);

    return 0;
}
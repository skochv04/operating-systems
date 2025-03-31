#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "settings.h"

// update the variable in signal handler
volatile bool close_flag = false;

void sig_handler(int signo){
    close_flag = true;
}

int main(int argc, char** argv) {
    // we need 4 arguments to start: the source file, name, address and server port
    if (argc < 4) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // register signal_handler
    signal(SIGINT, sig_handler);

    char* client_name = argv[1];
    // convert argument to the network byte order (uint32_t)
    uint32_t ip_address = inet_addr(argv[2]);
    // convert argument to long int uint16_t
    uint16_t port = (uint16_t)strtol(argv[3], NULL, 10);

    // store IPv4 address and port
    struct sockaddr_in addr = {
            .sin_family = AF_INET, // Internet Protocol version 4
            .sin_addr.s_addr = ip_address,
            .sin_port = htons(port) // convert to network byte order
    };

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) perror("connect");

    operation_message_t alive_message = {
            .operation = ALIVE
    };

    strncpy(alive_message.sender_client_id, client_name, MAX_CLIENT_ID_LENGTH);
    if(send(socket_fd, &alive_message, sizeof(alive_message), MSG_DONTWAIT) < 0) perror("send");

    pid_t pid = fork();
    if(pid < 0) perror("fork");
    else if (pid == 0){
        // child process
        while (!close_flag){
            operation_message_t message;
            if(recv(socket_fd, &message, sizeof(message), MSG_WAITALL) < 0) perror("recv");
            switch(message.operation){
                case LIST:
                    printf("Received LIST message\n");
                    printf("*** All clients list:\n");
                    for(int i = 0; i < message.payload.list.list_length; i++){
                        printf("%d: %s\n", i, message.payload.list.identifiers_list[i]);
                    }
                    break;
                case TO_ALL:
                    printf("Received TO_ALL message from: %s on date %s. Message: %s\n", message.sender_client_id, message.payload.to_all.current_date, message.payload.to_all.message);
                    break;
                case TO_ONE:
                    printf("Received TO_ONE message from: %s on date %s. Message: %s\n", message.sender_client_id, message.payload.to_all.current_date, message.payload.to_one.message);
                    break;
                case ALIVE:
                    send(socket_fd, &alive_message, sizeof(alive_message), MSG_DONTWAIT);
                    break;
                default:
                    printf("Invalid message type\n");
                    break;
            }
        }
    } else {
        // parent process is responsible for input
        char* operation_input_buffer = NULL;
        while (!close_flag){
            if(scanf("%ms", &operation_input_buffer) != 1) perror("input scanf");
            else {
                operation_message_t message;
                strncpy(message.sender_client_id, client_name, MAX_CLIENT_ID_LENGTH);

                if(strncmp(operation_input_buffer, "LIST", 4) == 0){
                    message.operation = LIST;
                    if (send(socket_fd, &message, sizeof(message), MSG_DONTWAIT) < 0) printf("send error");
                } else if (strncmp(operation_input_buffer, "2ALL", 4) == 0){
                    message.operation = TO_ALL;
                    scanf("%s", message.payload.to_all.message);
                    if (send(socket_fd, &message, sizeof(message), MSG_DONTWAIT) < 0) printf("send error");
                } else if (strncmp(operation_input_buffer, "2ONE", 4) == 0) {
                    message.operation = TO_ONE;
                    scanf("%s", message.payload.to_one.target_client_id);
                    scanf("%s", message.payload.to_one.message);
                    if (send(socket_fd, &message, sizeof(message), MSG_DONTWAIT) < 0) printf("send error");
                } else if (strncmp(operation_input_buffer, "STOP", 4) == 0){
                    close_flag = true;
                } else {
                    printf("Invalid message type\n");
                }

                free(operation_input_buffer);
            }

        }

        operation_message_t stop_messsage = {
                .operation = STOP
        };
        strncpy(stop_messsage.sender_client_id, client_name, MAX_CLIENT_ID_LENGTH);
        if (send(socket_fd, &stop_messsage, sizeof(stop_messsage), MSG_DONTWAIT) < 0) printf("send error");
    }

    close(socket_fd);

    return 0;
}
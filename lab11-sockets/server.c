#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>

#include "settings.h"

// update the variable in signal handler
volatile bool close_flag = false;

void sig_handler(int signo){
    close_flag = true;
}

void format_date(time_t timestamp, char *formatted_date) {
    struct tm *tm_info;
    tm_info = localtime(&timestamp);
    strftime(formatted_date, 11, "%d-%m-%Y", tm_info);
}

int main(int argc, char** argv){
    // we need 3 arguments to start: the source file, address and server port
    if (argc < 3) {
        printf("Can not proceed, not enough arguments\n");
        return 1;
    }

    // register signal_handler
    signal(SIGINT, sig_handler);

    // convert argument to the network byte order (uint32_t)
    uint32_t ip_address = inet_addr(argv[1]);
    // convert argument to long int uint16_t
    uint16_t port = (uint16_t)strtol(argv[2], NULL, 10);

    // store IPv4 address and port
    struct sockaddr_in addr = {
            .sin_family = AF_INET, // Internet Protocol version 4
            .sin_addr.s_addr = ip_address,
            .sin_port = htons(port) // convert to network byte order
    };

    int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    int t = 1; // 1 to "switch on" option
    // SOL_SOCKET - level, typical for settings
    // SO_REUSEADDR - option, which allows attaching socket to address which is being used by another connection
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int));

    // attach socket to address and port
    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) perror("bind");
    // allow socket to listen to clients
    if (listen(socket_fd, MAX_CLIENTS) < 0) perror("listen");

    int clients_fd[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) clients_fd[i] = -1;

    bool clients_id_set[MAX_CLIENTS] = {0}; // true if client is connected
    char clients_id_array[MAX_CLIENTS][MAX_CLIENT_ID_LENGTH] = {0};
    clock_t clients_alive_time[MAX_CLIENTS];

    clock_t ping_time = clock();

    while(!close_flag){
        int client_fd; // returned from accept function descriptor of the first connected client
        if((client_fd = accept(socket_fd, NULL, 0)) > 0){
            int i = 0;
            while (i < MAX_CLIENTS){
                if (clients_fd[i] == -1){
                    clients_fd[i] = client_fd;
                    clients_alive_time[i] = clock();

                    printf("Connected client %d\n", i);
                    break;
                }
                i++;
            }

            if (i == MAX_CLIENTS) printf("Clients max amount achieved!\n");
        }

        for (int i = 0; i < MAX_CLIENTS; i++){
            if (clients_fd[i] != -1){
                operation_message_t message;
                if(recv(clients_fd[i], &message, sizeof(message), MSG_DONTWAIT) > 0){
                    switch (message.operation) {
                        case LIST:
                            printf("Message from %s: LIST\n", message.sender_client_id);
                            int list_length = 0;
                            for (int j = 0; j < MAX_CLIENTS; j ++){
                                if (clients_fd[j] != -1){
                                    list_length++;
                                    strncpy(message.payload.list.identifiers_list[j], clients_id_array[j], MAX_CLIENT_ID_LENGTH);
                                }
                            }
                            message.payload.list.list_length = list_length;
                            send(clients_fd[i], &message, sizeof(message), MSG_DONTWAIT);
                            break;
                        case TO_ALL:
                            printf("Message from %s: TO_ALL\n", message.sender_client_id);
                            for (int j = 0; j < MAX_CLIENTS; j++)
                                if (i != j && clients_fd[j] != -1) {
                                    time_t current_time = time(NULL);
                                    format_date(current_time, message.payload.to_all.current_date);
                                    send(clients_fd[j], &message, sizeof(message), MSG_DONTWAIT);
                                }
                            break;
                        case TO_ONE:
                            printf("Message from %s: TO_ONE for client %s\n", message.sender_client_id, message.payload.to_one.target_client_id);
                            for (int j = 0; j < MAX_CLIENTS; j++)
                                if (clients_fd[j] != -1 && strncmp(clients_id_array[j], message.payload.to_one.target_client_id, MAX_CLIENT_ID_LENGTH) == 0){
                                    time_t current_time = time(NULL);
                                    format_date(current_time, message.payload.to_all.current_date);
                                    send(clients_fd[j], &message, sizeof(message), MSG_DONTWAIT);
                                }
                            break;
                        case ALIVE:
                            printf("Message from %s: CLIENT ALIVE\n", message.sender_client_id);
                            clients_alive_time[i] = clock();
                            if (!clients_id_set[i])
                                strncpy(clients_id_array[i], message.sender_client_id, MAX_CLIENT_ID_LENGTH);
                            break;
                        case STOP:
                            printf("Message from %s: CLIENT STOPPED\n", message.sender_client_id);
                            clients_fd[i] = -1;
                            clients_id_set[i] = false;
                            break;
                    }

                    fflush(stdout);
                }
            }
        }

        if ((clock() - ping_time) / CLOCKS_PER_SEC > 1){
            operation_message_t alive_message = {
                    .operation = ALIVE
            };
            for (int i = 0; i < MAX_CLIENTS; i++){
                if (clients_fd[i] != -1)
                    send(clients_fd[i], &alive_message, sizeof(alive_message), MSG_DONTWAIT);
            }
            ping_time = clock();
        }

        for (int i = 0; i < MAX_CLIENTS; i++){
            if (clients_fd[i] != -1 && (clock() - clients_alive_time[i]) / CLOCKS_PER_SEC > 5){
                printf("Client %s timed out\n", clients_id_array[i]);
                close(clients_fd[i]);
                clients_fd[i] = -1;
                clients_id_set[i] = false;
            }
        }

    }

    for (int i = 0; i < MAX_CLIENTS; i++){
        if(clients_fd[i] != -1) close(clients_fd[i]);
    }

    return 0;
}
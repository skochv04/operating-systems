#ifndef CHAT_SETTINGS_H
#define CHAT_SETTINGS_H

#define MAX_CLIENTS 5
#define SERVER_QUEUE_NAME "/chat_server_queue"
#define MESSAGE_MAXSIZE 2048
#define CLIENT_BUFFER_SIZE 50

// Enum for defining the type of the message

typedef enum{
    INIT, // initialize communication
    IDENTIFY, // sending client identification
    TEXT, // sending message text,
    FAIL, // failed to connect
    CLOSE // closure of client
} chat_message_type;

// Structure for the message
typedef struct {
    chat_message_type type;

    int identifier;
    char text[MESSAGE_MAXSIZE];
} chat_message;

#endif
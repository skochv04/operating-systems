#ifndef SETTINGS_H
#define SETTINGS_H

#define MAX_CLIENTS 40
#define MAX_CLIENT_ID_LENGTH 64
#define MAX_MESSAGE_LENGTH 128
#define DATE_LEN 10

typedef enum {
    LIST,
    TO_ALL,
    TO_ONE,
    STOP,
    ALIVE
} operation_type_t;

typedef struct {
    operation_type_t operation;
    char sender_client_id[MAX_CLIENT_ID_LENGTH];
    union {
        struct {
            char identifiers_list[MAX_CLIENTS][MAX_CLIENT_ID_LENGTH];
            int list_length;
        } list;
        struct{
            char target_client_id[MAX_CLIENT_ID_LENGTH];
            char message[MAX_MESSAGE_LENGTH];
            char current_date[DATE_LEN];
        } to_one;
        struct{
            char message[MAX_MESSAGE_LENGTH];
            char current_date[DATE_LEN];
        } to_all;
    } payload ;
} operation_message_t;

#endif // SETTINGS_H
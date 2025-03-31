#ifndef PRINT_SETTINGS_H
#define PRINT_SETTINGS_H
#include <semaphore.h>

#define USER_BUFFER 10
#define MAX_PRINTERS 256
#define MAX_PRINTER_BUFFER 256
#define SHARED_MEMORY "print_shared_memory"

typedef struct{
    sem_t printer_semaphore;
    char printer_buffer[MAX_PRINTER_BUFFER];
    size_t printer_buffer_size;
    bool isPrinting;
} printer;

typedef struct {
    printer printers[MAX_PRINTERS];
    int printers_amount;
} memory;

#endif // PRINT_SETTINGS_H
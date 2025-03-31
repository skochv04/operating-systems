
#include "zad2.h"

void mask()
{
    /*  Zamaskuj sygnał SIGUSR2, tak aby nie docierał on do procesu */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_SETMASK, &mask, NULL);
    check_mask();
}

void process()
{
    /*  Stworz nowy process potomny i uruchom w nim program ./check_fork
        W procesie macierzystym:
            1. poczekaj 1 sekundę
            2. wyślij SIGUSR1 do procesu potomnego
            3. poczekaj na zakończenie procesu potomnego */
    pid_t pid = fork();
    if (pid < 0){
        perror("failed to create fork");
        exit(1);
    }
    if (pid != 0){
        // parent process
        sleep(1);
        kill(pid, SIGUSR1);
        waitpid(pid, NULL, 0);
    }
    else{
        // child process
        execl("./check_fork", NULL);
    }
}

int main()
{
    mask();
    process();

    return 0;
}
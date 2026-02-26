#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void continueFunct(int sig)
{
    printf("The work will resume\n");
    sleep(1);
    printWorking();
    return;
}

void warning(int sig)
{

    printf("Press again CTRL C within 2 seconds to kill the process\n");
    signal(SIGALRM,continueFunct);
    signal(SIGINT,exit);
    alarm(2);
}

void exit(int sig)
{
    alarm(0);
    printf("GoodBye\n");
    sleep(1);
    exit(0);
}

void printWorking()
{
    while(1)
    {
        if(signal(SIGINT,warning))
            break;
        printf("working...\n");
        sleep(1);
    }
}

int main()
{
    printWorking();
}

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void printWorking(void);
void continueFunct(int sig);
void warning(int sig);
void exitProg(int sig);

int warned = 0;
int paused = 0;


void continueFunct(int sig)
{
	paused = 0;
	warned = 0;
	signal(SIGINT, warning);
    printf("The work will resume\n");
    sleep(1);
}

void warning(int sig)
{
    printf("Press again CTRL C within 2 seconds to kill the process\n");
	if(!warned)
	{
		paused = 1;
		warned = 1;
		signal(SIGINT, exitProg);
		signal(SIGALRM,continueFunct);
    	alarm(2);
	}
}

void exitProg(int sig)
{
    alarm(0);
    printf("GoodBye\n");
    exit(0);
}

int main(void)
{
    signal(SIGINT, warning);

    while (1)
    {
		if(!paused)
        	printf("working...\n");
        sleep(1);
    }
}

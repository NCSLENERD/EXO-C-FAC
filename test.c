#include <stdio.h>

int main(int argc, char *argv[]) {
    int i = 1;
	printf("%d\n",argc);
    while (argc != 1)
	{
		printf("%d\n",argv[i]);
		argc--;
		i++;
	}
    /*if(atoi(argv[1]) > 26 || atoi(argv[1]) < 4)
	{
		setenv("WIDTH",argv[1],1);
		P4WIDTH = atoi(getenv("WIDTH"));
	}
	else
		P4WIDTH = 16;*/

	return 0;
}
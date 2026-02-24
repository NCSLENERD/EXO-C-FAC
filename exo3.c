#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const char digits[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";

typedef struct {
    unsigned char base;   
    size_t size;          
    unsigned char *data;  
} number;


void free_number(number nbr)
{
    if (nbr.data != NULL) {
        free(nbr.data);
    }
}

number to_number(unsigned int nbr, unsigned char base)
{
	unsigned int stock = nbr;
	size_t i = 0;
    number n;
    n.data = NULL;
    n.size = 0;
    if (!(base == 2 || base == 4 || base == 8 ||
          base == 16 || base == 32 || base == 64)) {
        base = 16;
    }
    n.base = base;
    if (nbr == 0) {
        n.data = malloc(1);
        n.data[0] = 0;
        n.size = 1;
        return n;
    }
	while (stock > 0) {
        stock /= base;
        n.size++;
    }
	n.data = malloc(n.size * sizeof(unsigned char));
    while (nbr > 0 && i < n.size) {
        n.data[(n.size - i ) - 1] = nbr % base;
        nbr /= base;
        i++;
    }

    return n;
}

unsigned int to_uint(number nbr)
{
  unsigned int res = 0;
  unsigned int i = 0;
  size_t taille = nbr.size;
	
  while(taille > 0)
	{
		res += nbr.data[i] * (unsigned int)(pow(nbr.base,taille - 1));
		taille--;
		i++;
	}
	return res;
}

char* to_string(number nbr)
{
	char *res = malloc((nbr.size * sizeof(char)) + 1);
	if(nbr.size == 0)
	{
		res[0] = '\0';
		return res;
	} 
	  for (size_t i = 0; i < nbr.size; i++) {
        res[i] = digits[nbr.data[i]];  
    }
	res[nbr.size] = '\0';	
	return res;
}

int main(void)
{
    number n = to_number(0,2);
	for(size_t i = 0; i < n.size; i++)
	{
		printf("%c",digits[n.data[i]]);
	}
	printf("\n%u",to_uint(n));
	//printf("\n%u",(unsigned int)(10 + pow(10,2)));
	printf("\n");
	char *str = to_string(n);
	printf("%s",str);
	free(str);
	free_number(n);
    return 0;
}


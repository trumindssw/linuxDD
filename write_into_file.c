#include "header.h"

void write_into_file(FILE *fp,char *buff)
{
	if(fp==NULL)
	{
		printf("There is no such file existed\n");
	}
	fprintf(fp,"%d %s\n",getpid(),buff); 
}


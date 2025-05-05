#include "header.h"
void message1(char *message)
{

        FILE *fp=fopen("shem.txt","a");
        if(fp==NULL)
        {
                perror("file is not created");
                exit(1);
        }
        fprintf(fp, "Process ID: %d: %s\n", getpid(), message);
        fclose(fp);
}


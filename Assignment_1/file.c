#include "header.h" 

// Function to write a message to a file along with the process ID
void message1(char *message)
{
    // Open file "shem.txt" in append mode; create it if it doesn't exist
    FILE *fp = fopen("shem.txt", "a");
    
    // Check if file opening failed
    if (fp == NULL)
    {
        perror("file is not created");  // Print error message to stderr
        exit(1);                        // Exit program with error status
    }

    // Write process ID and message to the file
    fprintf(fp, "Process ID: %d: %s\n", getpid(), message);

    // Close the file to ensure data is saved
    fclose(fp);
}


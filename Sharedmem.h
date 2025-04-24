#include <sys/types.h>
 
#define SHM_KEY 0x1234
 
#define SEM_KEY 0x5678
 
#define MAX_MESSAGE_LEN 256
// Structure to hold the shared data 
typedef struct {
    char message[MAX_MESSAGE_LEN];
} SharedData;
 

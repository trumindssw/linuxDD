#include "shared.h"

key_t generate_shm_key() {
    // Use ftok with a file and a random id
    char *path = "/tmp"; // Use a common existing path
    int id = getpid() % 255;  // Or use rand() % 255

    return ftok(path, id);
}

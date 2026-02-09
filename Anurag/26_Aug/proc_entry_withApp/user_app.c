#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    FILE *fp;
    unsigned long my_jiffies, new_jiffies;
    char buf[128];

    while (1) {

        // for reading my jiffies value
        fp = fopen("/proc/tss", "r");
        if (!fp) {
            perror("fopen");
            return 1;
        }

        // reading line by line and parse my_jiffies
        while (fgets(buf, sizeof(buf), fp)) {
            if (sscanf(buf, "my_jiffies: %lu", &my_jiffies) == 1)
                break;
        }
        fclose(fp);

        // Add 100
        printf("Got the old jiffies value : %lu , adding 100 to it\n",my_jiffies);
        new_jiffies = my_jiffies + 100;

        
        // after adding 100 have to add it to the /proc/tss 
        fp = fopen("/proc/tss", "w");
        if (!fp) {
            perror("fopen write");
            return 1;
        }
        // writing to the /proc/tss
        fprintf(fp, "%lu", new_jiffies);

        fclose(fp);

        printf("my_jiffies=%lu, my_new_jiffies=%lu\n", my_jiffies, new_jiffies);

        sleep(1);
    }

    return 0;
}

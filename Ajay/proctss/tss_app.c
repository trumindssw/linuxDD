#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    while (1) {
        FILE *f = fopen("/proc/tss", "r");
        if (!f) { perror("open"); return 1; }

        char line[128];
        unsigned long my_jiffies = 0;
        while (fgets(line, sizeof(line), f)) {
            if (sscanf(line, "my_jiffies: %lu", &my_jiffies) == 1) {
                break;
            }
        }
        fclose(f);

        unsigned long newj = my_jiffies + 100;

        f = fopen("/proc/tss", "w");
        if (!f) { perror("write"); return 1; }
        fprintf(f, "%lu\n", newj);
        fclose(f);

        printf("Read my_jiffies=%lu, wrote my_new_jiffies=%lu\n", my_jiffies, newj);
        sleep(1);
    }
    return 0;
}

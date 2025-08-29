#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned long read_my_jiffies(void)
{
    FILE *f = fopen("/proc/tss", "r");
    if (!f) {
        perror("open /proc/tss");
        return 0;
    }

    char line[256];
    unsigned long val = 0;

    /* Look for the line: "my_jiffies: <num>" */
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "my_jiffies: %lu", &val) == 1) {
            break;
        }
    }
    fclose(f);
    return val;
}

static int write_my_new_jiffies(unsigned long v)
{
    FILE *f = fopen("/proc/my_new_jiffies", "w");
    if (!f) {
        perror("open /proc/my_new_jiffies");
        return -1;
    }
    fprintf(f, "%lu\n", v);
    int rc = ferror(f) ? -1 : 0;
    fclose(f);
    return rc;
}

int main(void)
{
    while (1) {
        unsigned long mj = read_my_jiffies();
        if (mj == 0) {
            /* read failed or zero at boot; keep trying */
        }
        unsigned long newv = mj + 100;

        if (write_my_new_jiffies(newv) == 0) {
            printf("Set my_new_jiffies = %lu (from my_jiffies %lu + 100)\n", newv, mj);
        }
        sleep(1);
    }
    return 0;
}



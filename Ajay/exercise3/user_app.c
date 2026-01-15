#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>

#define MYDRV_ENABLE_TIMER _IO('a', 1)
#define MYDRV_SET_TIMER_VAL _IOW('a', 2, int)
#define MYDRV_STOP_TIMER _IO('a', 3)

static volatile sig_atomic_t stop = 0;
static void handle_sigint(int sig) { (void)sig; stop = 1; }

int main(int argc, char **argv)
{
    int fd, val = 2;
    char buf[256];
    FILE *p;

    if (argc >= 2) {
        int t = atoi(argv[1]);
        if (t >= 0) val = t;
    }

    signal(SIGINT, handle_sigint);

    fd = open("/dev/mydrv", O_RDWR);
    if (fd < 0) { perror("open /dev/mydrv"); return 1; }

    if (ioctl(fd, MYDRV_SET_TIMER_VAL, &val) < 0) {
        perror("ioctl SET_TIMER_VAL");
        close(fd);
        return 1;
    }

    printf("Tasklet delay = %d s. Ctrl+C to stop.\n", val);

    while (!stop) {
        p = popen("date '+%F %T %z (user)\\n'", "r");
        if (p && fgets(buf, sizeof(buf), p)) {
            write(fd, buf, strlen(buf));
            printf("Wrote user line\n");
        }
        if (p) pclose(p);

        if (ioctl(fd, MYDRV_ENABLE_TIMER) < 0) perror("ioctl ENABLE_TIMER");
        else printf("ENABLE_TIMER sent\n");

        sleep(10);
    }

    if (ioctl(fd, MYDRV_STOP_TIMER) < 0) perror("ioctl STOP_TIMER");
    else printf("Timer stopped\n");

    close(fd);
    return 0;
}

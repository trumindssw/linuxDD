// UserApp

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#define DEVICE "/dev/my_drv_"

struct ioctl_arg {
    char path[256];
    int tz_offset;   // using it to UTC -> IST
    int setval;
};

#define MYDRV_ENABLE_TIMER  _IO('M', 1)
#define MYDRV_SET_TIMER_VAL _IOW('M', 2, unsigned int)
#define MYDRV_STOP_TIMER    _IO('M', 3)

    int fd;
    bool loop_running = false;

void *enable_loop(void *arg) {
    while (loop_running) {
        if (ioctl(fd, MYDRV_ENABLE_TIMER) < 0) {
            perror("ioctl ENABLE_TIMER");
        } else {
            printf("Sent ENABLE_TIMER\n");
        }

        printf("waiting for 10s to trigger the driver again\n\n");
        sleep(10);

    }
    return NULL;
}

int main()
{
    int i;
    struct ioctl_arg arg;
    
    time_t now = time(NULL);
    struct tm local_tm = *localtime(&now);
    struct tm gm_tm = *gmtime(&now);


    strcpy(arg.path, "/tmp/datetime.txt");
    arg.setval = 5;
    // compute offset (local - UTC) in seconds
    arg.tz_offset = (local_tm.tm_hour - gm_tm.tm_hour) * 3600 +
                    (local_tm.tm_min - gm_tm.tm_min) * 60;

    // handle day wraparound
    if (local_tm.tm_yday != gm_tm.tm_yday) {
        if (local_tm.tm_yday > gm_tm.tm_yday)
            arg.tz_offset += 24 * 3600;
        else
            arg.tz_offset -= 24 * 3600;
    }
    
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open device");
        return 1; 
    }
    
    printf("\nSetting timer value to 5s\n\n");

    if (ioctl(fd, MYDRV_SET_TIMER_VAL, &arg) < 0) {
        perror("ioctl SET_TIMER_VAL");
        close(fd);
        return 1;
    }

    int choice, val;
    bool running = true;
    pthread_t enable_thread;

    while (running) {
        printf("Press 1 to ENABLE_TIMER\n");
        printf("Press 2 to Stop Timer\n");
        printf("Press 3 to Exit\n\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1:
            if (loop_running) {
                printf("Timer is enabled already\n");
                break;
            }
            loop_running = true;
            pthread_create(&enable_thread, NULL, enable_loop, NULL);
            break;

        case 2:
            if (ioctl(fd, MYDRV_STOP_TIMER) < 0) {
                perror("ioctl STOP_TIMER");
            } else {
                printf("STOP_TIMER sent, stopping loop...\n\n");
                loop_running = false;
            }
            break;

        case 3:
            printf("Exiting...\n");
            loop_running = false;
            running = false;
            break;

        default:
            printf("Invalid choice!\n");
        }
    }

    close(fd);

    printf("Writing done into file /tmp/datetime.txt");
    
    return 0;
}

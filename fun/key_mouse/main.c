#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <linux/input.h>
#include <errno.h>

// /proc/bus/input/devices

#define KEYBOARDFILE "/dev/input/event3"
#define MOUSEFILE "/dev/input/event2"

int main(int argc, char **argv)
{
    int activity;
    int mouse_fd, keyboard_fd, max_fd;
    struct input_event ie;
    fd_set readfds;
    struct timeval timeout;

    if((mouse_fd = open(MOUSEFILE, O_RDONLY)) == -1)
    {
        perror("Cannot access mouse device");
        exit(EXIT_FAILURE);
    }

    if((keyboard_fd = open(KEYBOARDFILE, O_RDONLY)) == -1)
    {
        perror("Cannot access keyboard device");
        exit(EXIT_FAILURE);
    }

    max_fd = keyboard_fd;

    int flags = fcntl(mouse_fd, F_GETFL, 0);
    fcntl(mouse_fd, F_SETFL, flags | O_NONBLOCK);

    while(1)
    {
        FD_ZERO(&readfds);
        // FD_SET(mouse_fd, &readfds);
        FD_SET(keyboard_fd, &readfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        activity = select(max_fd + 1 , &readfds , NULL , NULL , &timeout);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error\n");
            continue;
        }

        if (!activity /*timeout*/)
        {
            if (read(mouse_fd, &ie, sizeof(struct input_event)) > 0)
            {
                printf("MOUSE: %d, %d, %d\n", ie.type, ie.value, ie.code);
            }
        } else
        if (FD_ISSET(keyboard_fd, &readfds)) 
        {
            if (read(keyboard_fd, &ie, sizeof(struct input_event)) > 0)
            {
                printf("KEY: %d, %d, %d\n", ie.type, ie.value, ie.code);
            }
            if (read(mouse_fd, &ie, sizeof(struct input_event)) > 0)
            {
                printf("MOUSE: %d, %d, %d\n", ie.type, ie.value, ie.code);
            }
        }
    }

    close(mouse_fd);
    close(keyboard_fd);

    return 0;
}

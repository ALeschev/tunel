#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <linux/input.h>
#include <errno.h>

#define KEYBOARDFILE "/dev/input/event4"
#define MOUSEFILE "/dev/input/event5"

int main(int argc, char **argv)
{
    int activity;
    int mouse_fd, keyboard_fd, max_fd;
    struct input_event ie;
    fd_set readfds;

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

    max_fd = (mouse_fd > keyboard_fd)? mouse_fd:keyboard_fd;

    КАРОЧ
    МЫШКУ ЧИТАЕМ ТОЛЬКО ПРИ НАЖАТИИ КЛАВИШИ ИЛИ ПО ТАЙМАУТУ СЕЛЕКТА
    ИНАЧЕ ПОЕЗДА. СЛИШКОМ МНОГО ЭВЕНТОВ

    // while(1)
    // {
    //     FD_ZERO(&readfds);
    //     FD_SET(mouse_fd, &readfds);
    //     FD_SET(keyboard_fd, &readfds);

    //     activity = select(max_fd + 1 , &readfds , NULL , NULL , NULL);
    //     if ((activity < 0) && (errno != EINTR)) 
    //     {
    //         printf("select error\n");
    //         continue;
    //     }

    //     if (FD_ISSET(mouse_fd, &readfds)) 
    //     {
            if (read(mouse_fd, &ie, sizeof(struct input_event)))
            {
                printf("MOUSE: %d, %d, %d\n", ie.type, ie.value, ie.code);
            }
        // } else
        // if (FD_ISSET(keyboard_fd, &readfds)) 
        // {
        //     if (read(keyboard_fd, &ie, sizeof(struct input_event)))
        //     {
        //         // printf("KEY: %d, %d, %d\n", ie.type, ie.value, ie.code);
        //     }
        // }
    // }

    close(mouse_fd);
    close(keyboard_fd);

    return 0;
}

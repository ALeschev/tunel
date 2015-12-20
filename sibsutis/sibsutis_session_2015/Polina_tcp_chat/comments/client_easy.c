#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <errno.h>

#include "logger.h"

/* сначала прочитай server_easy.c */
/* тут должно быть проще :) */

int main(int argc, char *argv[])
{
    int sockfd, portno, result;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set readfds;
    char buffer[256];

    if (argc < 3)
       log_print(eLOG_ERR, "Usage %s hostname port", argv[0]);

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        log_print(eLOG_ERR, "Socket: %s", strerror(errno));

    /* пытаемся получить ип по доменному имени 
     * т.е. спокойно можешь вводить yandex.ru,
     * если поднимишь этот чат-сервер у них на серверах ;)
     * ну можно просто ип написать, эта штука всеядная
     */
    server = gethostbyname(argv[1]);
    if (server == NULL)
        log_print(eLOG_ERR, "Resolv '%s' failed", argv[1]);

    /* выставили ип + порт сервера */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    /* пытаемся подключиться к серверу */
    result = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (result < 0) 
        log_print(eLOG_ERR, "Connect: %s", strerror(errno));

    log_print(eLOG_INFO, "Connection successful: %s:%u",
              inet_ntoa(serv_addr.sin_addr),
              ntohs(serv_addr.sin_port));

    /* главный цикл. выход из него только по команде exit в консоли */
    while (1)
    {
        /* та же самая байда с множеством, что и в сервере */
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);

        memset(buffer, 0, sizeof (buffer));

        /* та же самая байда, что и в сервере */
        result = select(sockfd + 1 , &readfds , NULL , NULL , NULL);
        if ((result < 0) && (errno != EINTR)) 
        {
            log_print(eLOG_ERR, "Select: %s", strerror (errno));
            break;
        }

        /* проверяем "а может новая команда пришла?" */
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            /* прочитали то, что кто-то ввел в консоли */
            result = read(STDIN_FILENO, buffer, sizeof (buffer));
            if (result <= 0)
            {
                log_print(eLOG_WARN, "Read stdin: %s\n",
                          strerror (errno));
            } else
            /* это защита от тех, кто просто так любит долбить по ентеру */
            if (buffer[0] != '\n')
            {
                /* получили exit - закрываемся */
                if (!strncmp(buffer, "exit", 4))
                {
                    log_print(eLOG_WARN, "Exit by command");
                    exit(0);
                }

                /* отправляем серверу то, что прочитали из консоли */
                write(sockfd, buffer, strlen(buffer));
            }
        } else
        /* что-то пришло от сервера */
        if (FD_ISSET(sockfd, &readfds))
        {
            result = read(sockfd, buffer, sizeof (buffer));
            if (result <= 0)
            {
                log_print(eLOG_WARN, "Server closed the connection");
                break;
            }
            else
            {
                /* пишем в консоль то, что прочитали */
                log_print(eLOG_INFO, "Recv: %.*s", result, buffer);
            }
        }
    }

    /* если мы тут, значит кто-то ввел exit */

    close(sockfd);
    return 0;
}
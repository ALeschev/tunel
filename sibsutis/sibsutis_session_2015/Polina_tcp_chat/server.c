#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>

#include "logger.h"

/* Основная структура для хранения информации о клиенте */
typedef struct client
{
    struct sockaddr_in addr;
    int addrlen;
    int sockfd;
    struct client *next;
} client_t;

void send_broadcast_message(client_t *clients_list, int ignore_fd,
                            char *buff, int len)
{
    client_t *p_client = clients_list;

    /* обход списка клиентов и отправка сообщения */
    while (p_client)
    {
        if (!p_client->sockfd || p_client->sockfd == ignore_fd)
        {
            p_client = p_client->next;
            continue;
        }

        write(p_client->sockfd, buff, len);

        p_client = p_client->next;
    }
}

void accept_new_client(client_t *clients_list, int *accept_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int newsockfd;
    client_t *p_client = clients_list;
    char buff[256] = "Hello from server!";

    clilen = sizeof(cli_addr);

    /* прием пнового подключения */
    newsockfd = accept(*accept_sockfd,
                        (struct sockaddr *)&cli_addr,
                        &clilen);
    if (newsockfd < 0)
        log_print(eLOG_ERR, "Accept: %s", strerror(errno));

    log_print(eLOG_INFO, "Accept new conection: %s:%u",
              inet_ntoa(cli_addr.sin_addr),
              ntohs(cli_addr.sin_port));

    /* добавление нового клиента в конец списка */
    while (p_client->next)
        p_client = p_client->next;

    p_client->next = (client_t *)calloc(1, sizeof (client_t));
    if (!p_client)
    {
        log_print(eLOG_WARN, "Failed to add new client: %s", strerror(errno));
        close(newsockfd);
        return;
    }

    p_client = p_client->next;
    p_client->next = NULL;

    memcpy(&p_client->addr, &cli_addr, sizeof (p_client->addr));
    p_client->addrlen = clilen;
    p_client->sockfd = newsockfd;

    log_print(eLOG_INFO, "Added new client: %s:%d socket %d",
              inet_ntoa(cli_addr.sin_addr),
              ntohs(cli_addr.sin_port),
              newsockfd);

    write(newsockfd, buff, sizeof (buff));

    sprintf(buff, "%s:%u connected.",
            inet_ntoa(p_client->addr.sin_addr),
            ntohs(p_client->addr.sin_port));

    send_broadcast_message(clients_list,
                           p_client->sockfd,
                           buff, strlen(buff));
}

void proc_client_message(client_t *clients_list, fd_set *rfds)
{
    int result;
    char buff[1024] = {0};
    client_t *p_client = clients_list;
    client_t *prev_client = p_client;

    /* поиск клиента от которого пришло сообщение */
    while (p_client)
    {
        if (p_client->sockfd && FD_ISSET (p_client->sockfd, rfds))
        {
            result = read(p_client->sockfd, buff, sizeof (buff));
            if (result)
            {
                log_print(eLOG_INFO, "%s:%u say: %.*s",
                          inet_ntoa(p_client->addr.sin_addr),
                          ntohs(p_client->addr.sin_port),
                          result, buff);

                send_broadcast_message(clients_list,
                                       p_client->sockfd,
                                       buff, result);
            }
            else
            {
                /* обработка разъединения клиента */
                sprintf(buff, "%s:%u disconnected.",
                        inet_ntoa(p_client->addr.sin_addr),
                        ntohs(p_client->addr.sin_port));

                log_print(eLOG_WARN, "%s", buff);

                send_broadcast_message(clients_list,
                                       p_client->sockfd,
                                       buff, strlen(buff));

                close(p_client->sockfd);
                prev_client->next = p_client->next;
                free(p_client);
            }

            break;
        }

        prev_client = p_client;
        p_client = p_client->next;
    }
}

void update_rfds(fd_set *rfds, client_t *clients_list,
                 int accept_sockfd, int *max_fd)
{
    int max_list_fd = 0;
    client_t *p_client = clients_list;

    FD_ZERO(rfds);
    FD_SET (STDIN_FILENO, rfds);
    FD_SET(accept_sockfd, rfds);

    /* обновление дескрипторов в множестве rfds */
    while (p_client)
    {
        if (p_client->sockfd)
        {
            FD_SET(p_client->sockfd, rfds);

            if (max_list_fd < p_client->sockfd)
                max_list_fd = p_client->sockfd;
        }

        p_client = p_client->next;
    }

    if (accept_sockfd < max_list_fd)
        *max_fd = max_list_fd;
    else
        *max_fd = accept_sockfd;
}

int main(int argc, char *argv[])
{
    int accept_sockfd, portno;
    struct sockaddr_in serv_addr;
    fd_set rfds;
    int retval, max_fd;
    client_t clients_list, *p_client;
    char buff[1024] = {0};

    if (argc < 2)
        log_print(eLOG_ERR, "Usage: %s <port>", argv[0]);

    accept_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (accept_sockfd < 0)
        log_print(eLOG_ERR, "Socket: %s", strerror(errno));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    retval = bind(accept_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (retval < 0)
        log_print(eLOG_ERR, "Bind: %s", strerror(errno));

     listen(accept_sockfd, 5);

     max_fd = accept_sockfd;
     memset(&clients_list, 0, sizeof (clients_list));

     log_print(eLOG_INFO, "Server started");

     while (1)
     {
        update_rfds(&rfds, &clients_list, accept_sockfd, &max_fd);

        retval = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if ((retval < 0) && (errno != EINTR))
            log_print(eLOG_ERR, "Select: %s", strerror(errno));

        if (FD_ISSET (accept_sockfd, &rfds))
        {
            /* обработка запроса на новое подключение */
            accept_new_client(&clients_list, &accept_sockfd);
        } else
        if (FD_ISSET (STDIN_FILENO, &rfds))
        {
            /* обработка команд из консоли */
            retval = read (STDIN_FILENO, buff, sizeof(buff));
            if (retval <= 0)
            {
                log_print(eLOG_WARN, "Read stdin: %s\n",
                          strerror (errno));
            } else
            if (buff[0] != '\n')
            {
                if (!strncmp(buff, "exit", 4))
                {
                    log_print(eLOG_WARN, "Exit by command");
                    break;
                }
                else
                {
                    log_print(eLOG_WARN, "Skip unknown command");
                }
            }
        }
        else
        {
            /* обработка сообщений от клиентов */
            proc_client_message(&clients_list, &rfds);
        }
    }

    p_client = &clients_list;
    while (p_client)
    {
        if (p_client->sockfd)
            close(p_client->sockfd);

        p_client = p_client->next;
    }


    close(accept_sockfd);
    return 0; 
}
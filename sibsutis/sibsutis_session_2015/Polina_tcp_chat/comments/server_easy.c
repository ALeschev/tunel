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

/* Сруктура описывающая клиента.
 * Надеюсь у вас уже были односвязные списки?
 * Знакомься, это элемент двусвязного списка.
 * Когда подключается новый клиент, то заполняется вот такая структура
 * и помещается в список.
 * Т.е. все подключенные к нам клиенты - это один длинный список
 */
typedef struct client
{
    struct sockaddr_in addr; /* собственно сам адрес клиента - ип + порт */
    int addrlen;             /* длина адреса, в общем-то очень нужная штука */
    int sockfd;              /* сам сокет в сторону клиента */
    struct client *next;     /* указатель на следующего клиента */
} client_t;

/* Эта функция бегает по списку клиентов и отправляет сообщения всем, кроме ignore_fd */
void send_broadcast_message(client_t *clients_list, int ignore_fd,
                            char *buff, int len)
{
    client_t *p_client = clients_list;

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

/* Функция подключения нового клиента */
void accept_new_client(client_t *clients_list, int *accept_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int newsockfd;
    client_t *p_client = clients_list;
    char buff[256] = "Hello from server!";

    clilen = sizeof(cli_addr);

    /* собственно вот тут мы и принимаем новое подключение*/
    newsockfd = accept(*accept_sockfd,
                        (struct sockaddr *)&cli_addr,
                        &clilen);
    if (newsockfd < 0)
        log_print(eLOG_ERR, "Accept: %s", strerror(errno));

    log_print(eLOG_INFO, "Accept new conection: %s:%u",
              inet_ntoa(cli_addr.sin_addr),
              ntohs(cli_addr.sin_port));

    /* бежим в самый конец списка клиентов, чтобы воткнуть туда нового */
    while (p_client->next)
        p_client = p_client->next;

    /* выделяем память под нового клиента*/
    p_client->next = (client_t *)calloc(1, sizeof (client_t));
    if (!p_client)
    {
        log_print(eLOG_WARN, "Failed to add new client: %s", strerror(errno));
        close(newsockfd);
        return;
    }
    /* стыкаем его */
    p_client = p_client->next;
    p_client->next = NULL;

    /* заполняем */
    memcpy(&p_client->addr, &cli_addr, sizeof (p_client->addr));
    p_client->addrlen = clilen;
    p_client->sockfd = newsockfd;

    log_print(eLOG_INFO, "Added new client: %s:%d socket %d",
              inet_ntoa(cli_addr.sin_addr),
              ntohs(cli_addr.sin_port),
              newsockfd);

    /* говорим ему "привет" */
    write(newsockfd, buff, sizeof (buff));

    sprintf(buff, "%s:%u connected.",
            inet_ntoa(p_client->addr.sin_addr),
            ntohs(p_client->addr.sin_port));

    /* отправляем всем клиентам сообщение о том, что подключился новый клиент */
    send_broadcast_message(clients_list,
                           p_client->sockfd,
                           buff, strlen(buff));
}

/* функция для обработки сообщений от клиентов*/
void proc_client_message(client_t *clients_list, fd_set *rfds)
{
    int result;
    char buff[1024] = {0};
    client_t *p_client = clients_list;
    client_t *prev_client = p_client;

    /* бегаем по списку и ищем от какого же это клиента нам такое пришло... */
    while (p_client)
    {
        if (p_client->sockfd && FD_ISSET (p_client->sockfd, rfds))
        {
            /* нашли клиента и пытаемся прочитать чтоже он нам прислал */
            result = read(p_client->sockfd, buff, sizeof (buff));
            if (result)
            {
                log_print(eLOG_INFO, "%s:%u say: %.*s",
                          inet_ntoa(p_client->addr.sin_addr),
                          ntohs(p_client->addr.sin_port),
                          result, buff);

                /* отправляем то, что прислал клиент всем остальным */
                send_broadcast_message(clients_list,
                                       p_client->sockfd,
                                       buff, result);
            }
            else
            {
                /* а вот тут беда, прочитать не смогли, а это 150% что клиент отключился */
                sprintf(buff, "%s:%u disconnected.",
                        inet_ntoa(p_client->addr.sin_addr),
                        ntohs(p_client->addr.sin_port));

                log_print(eLOG_WARN, "%s", buff);

                /* говорим всем, что клиента с нами больше нет */
                send_broadcast_message(clients_list,
                                       p_client->sockfd,
                                       buff, strlen(buff));

                /* удаляем его из списка и подчищаем память */
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

/* Такс, а вот эта специфичная штука:
 * все сокеты у нас хранятся в множестве (х3 как правильно оно называется) rfds
 * и каждый раз когда мы получаем что-то из сокета в этом множестве, т.е.
 * выходим из системной ф-и select нам КАЖДЫЙ раз блин приходися это множество
 * перезаполнять. Такие дела.
 */
void update_rfds(fd_set *rfds, client_t *clients_list,
                 int accept_sockfd, int *max_fd)
{
    int max_list_fd = 0;
    client_t *p_client = clients_list;

    FD_ZERO(rfds);               /* зануляем */
    FD_SET (STDIN_FILENO, rfds); /* добавляем stdin сокет, для чтения из консоли */
    FD_SET(accept_sockfd, rfds); /* добавляем главный сокет который слушает подключения */

    /* бегаем по списку клиентов и добавляем их сокеты */
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

    /* в системную ф-ю select нужно передавать максимальный дескриптор
     * выше мы его нашли, а тут запомнили
     */
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
    client_t clients_list, *p_client; /* clients_list - сам список плиентов */
    char buff[1024] = {0};

    if (argc < 2)
        log_print(eLOG_ERR, "Usage: %s <port>", argv[0]);

    /* создали сокет. SOCK_STREAM - это как раз TCP */
    accept_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (accept_sockfd < 0)
        log_print(eLOG_ERR, "Socket: %s", strerror(errno));

    /* выставили ип + порт сервера */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* прибиндили сокет к адресу */
    retval = bind(accept_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (retval < 0)
        log_print(eLOG_ERR, "Bind: %s", strerror(errno));

    /* слушаем :) */
     listen(accept_sockfd, 5);

     max_fd = accept_sockfd;
     memset(&clients_list, 0, sizeof (clients_list));

     log_print(eLOG_INFO, "Server started");

     /* главный цикл. выход из него только по команде exit в консоли */
     while (1)
     {
        /* обновляем дескрипторы */
        update_rfds(&rfds, &clients_list, accept_sockfd, &max_fd);

        /* сама ф-я селект, она ждет события на одном из сокетов в множестве rfds  */
        retval = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if ((retval < 0) && (errno != EINTR))
            log_print(eLOG_ERR, "Select: %s", strerror(errno));

        /* проверяем "а не запрос ли это на новое подключение?" */
        if (FD_ISSET (accept_sockfd, &rfds))
        {
            accept_new_client(&clients_list, &accept_sockfd);
        } else
        /* проверяем "а может новая команда пришла?" */
        if (FD_ISSET (STDIN_FILENO, &rfds))
        {
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
            /* ни то, ни это - значи что-то пришло о клиента */
            proc_client_message(&clients_list, &rfds);
        }
    }

    /* если мы тут, значит кто-то ввел exit
     * бегаем по списку и закрываем сокеты клиентов, свой сокет и выходим
     */
    p_client = &clients_list;
    while (p_client)
    {
        if (p_client->sockfd)
            close(p_client->sockfd);

        /* free memory is meaningless */

        p_client = p_client->next;
    }


    close(accept_sockfd);
    return 0; 
}
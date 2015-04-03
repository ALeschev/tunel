#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define UNSPEC_PROTO 0

int main(int argc, const char *argv[])
{ 
    if (argc != 2) {
        printf("usage: routetracer <ip address or hostname>\n");
        return -1;
    }

    struct addrinfo hints; //params for ret val of getaddrinfo
    struct addrinfo* ret; //return value of getaddrinfo
    struct sockaddr* reply_addr;
    char ipv4[INET_ADDRSTRLEN];
    char* msg = "THE PORT IS OVER 9000!!!!";
    int status = 0;
    int ttl = 0;
    int src_sock = 0;
    int recv_sock = 0;
    socklen_t reply_addr_len = sizeof(struct sockaddr);
    const char* dest_port = "9001";
    int icmp_msg_len = 100;
    char icmp_msg[icmp_msg_len];

    //define what we want from getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP packets

    //call getaddrinfo to fill ret, w/ error chk
    if ((status = getaddrinfo(argv[1], dest_port, &hints, &ret)) != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    //extract IPv4 address from ret
    struct sockaddr_in* ip = (struct sockaddr_in *)ret->ai_addr;

    //convert address from pure numbers to something easier to read
    inet_ntop(ret->ai_family, &(ip->sin_addr), ipv4, INET_ADDRSTRLEN);

    //kindly inform the user of which hostname they are connecting to
    printf("Route for: %s\n", ipv4);

    //create a socket for our machine
    if ((src_sock = socket(ret->ai_family, ret->ai_socktype, 
                    ret->ai_protocol)) < 0) {
        fprintf(stderr, "Error creating host socket: %s\n", strerror(errno));
        return -1;
    }

    //create a socket to recv icmp packet from hops 
    if ((recv_sock = socket(AF_INET, SOCK_DGRAM, UNSPEC_PROTO)) < 0){
        fprintf(stderr, "Error creating recv socket: %s\n", strerror(errno));
    }

    /*
     * We go from hop to hop by incrementing the time to live in the IP header
     * for each hop we visit until we reach the destination IP address (which we
     * already have). Time to live decrements for every hop, so once it reaches
     * zero we report the IP address of the node we are connected to.
     */

    //while(test_ip != dest_ip)
    //time_to_live++
    //send_to(dest_addr)
    //receive icmp error message
    //get src addr of error msg from ip header of icmp
    //test_ip = src addr
    /*
    while (last_hop == 0) {
        ttl++;
        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)ip, sizeof(ip));
    }
    */

    ttl = 1;
    if (!(setsockopt(src_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)))) {
        printf("TTL set successfully\n");
    } else {
        printf("Error setting TTL: %s\n", strerror(errno));
    }

    if ((sendto(src_sock, msg, strlen(msg), 0, ret->ai_addr, 
                    ret->ai_addrlen)) > 0) {
        printf("msg sent successfully\n");
    } else {
        fprintf(stderr, "Error sending msg: %s\n", strerror(errno));
    }

    if ((recvfrom(recv_sock, icmp_msg, icmp_msg_len, 0, reply_addr, 
                    &reply_addr_len)) != -1) {
        /* PROCESS THE INFORMATION */
        printf("Packet received\n");
    } else {
        fprintf(stderr, "Error receiving packet: %s\n", strerror(errno));
    }

    return 0;
}
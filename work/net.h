
#ifndef _NET_HEADER_H_
#define _NET_HEADER_H_

#include <netdb.h>
#include "sipt_def.h"

int 	net_init(int idx);
void	net_close(int idx);
int 	net_init_iface(int id, int vid, int alias);
int 	net_close_iface(int id);

char	*net_ip(int idx);
struct	in_addr net_addr(int idx);
struct	in_addr net_addr_by_name(const char *iface_name);
struct	in_addr net_iface_addr(int id);
int 	net_mac(int idx, unsigned char *mac_id);

int 	net_check_iface(char *if_name, uint32_t *ipaddr);
int 	net_is_ipaddr_local(uint32_t ip, int *vidx);
int 	if_mac_lookup(int idx, int iface_idx, struct in_addr ipaddr, unsigned char *mac_id, int *vidx);
int 	ping(int iface_idx, struct in_addr ipaddr);

void 	net_tick(void);
int  	net_check(int idx);

void 	hostent_clear(struct hostent *hp, int stage);
void 	srv_hostent_clear(struct _srv_host_entry *hp, size_t nmemb);
int  	udns_gethostbyname (struct hostent *hp, const char *name, int *ttl);

struct 	_sipt_intf_info;
int  	udns_gethostbyname_udp_srv (struct _sipt_intf_info *info, const char *service);
int  	udns_gethostbyname_tcp_srv (struct _sipt_intf_info *info, const char *service);

int net_init_route (int idx);
int net_close_route (int idx);
int net_update_route (int idx);

#define SERVICE_SIP "sip"
#define SERVICE_H323 "h323"
#define PROTOCOL_UDP "udp"
#define PROTOCOL_TCP "tcp"

char 	*net2str(struct in_addr ipaddr, char *s, int ssize);

#endif /*_NET_HEADER_H_*/


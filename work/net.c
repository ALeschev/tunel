
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>


#include "ut/ut_queues.h"
#include "ut/ut_port.h"
#include "ut/ut_thread.h"
#include "ut/ut_sema.h"

#include "proc.h"
#include "bcktrace.h"
#include "config_proc.h"

#include "net.h"
#include "msp_conn.h"
#include "sipt_cmd.h"
#include "h323_proc_f.h"

/*-----------------------------------------------------------------------*/

#define MAX_NUM_IFREQ 512

#define MAX_IF 2

typedef struct s_route_entry {
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	int vidx;
	int flags;
	char dev_name[20];
} route_entry;

struct iface_t
{
	char if_name[32];
	char ipaddr_str[32];
	struct in_addr netmask;
	struct sockaddr_in ipaddr;

	/*defaults routes*/
	route_entry gw_route;
	route_entry subnet_route;
};

typedef struct _net_if
{
	struct iface_t iface[NET_IFACE_MAX_SMG];

	char local_mac_buf[20];

	route_entry route[NETWORK_MAX_ROUTES];
} net_if_t;

/*-----------------------------------------------------------------------*/

char eth1_mac[] = {0x02, 0x00, 0x04, 0x00, 0x00, 0x02};

static ut_thread_t *net_thread = NULL;
static int net_running = 0;
static void *net_task(void *ptr);

static net_if_t  net_if[MAX_IF];

#define PROC_ROUTE	"/proc/net/route"

static int trace_net = 0;

/*-----------------------------------------------------------------------*/

#include "net_gethostbyname.c"

/*-----------------------------------------------------------------------*/

int SetTraceNet(int lvl)
{
	if(lvl >=0 ) trace_net = lvl;

	return trace_net;
}

/*-----------------------------------------------------------------------*/

int TraceNet(void)
{
	if(!Trace()) return 0;
	return trace_net;
}

/*-----------------------------------------------------------------------*/

static unsigned long iptohl(struct in_addr ip)
{
	return (ntohl(ip.s_addr));
}

/*-----------------------------------------------------------------------*/

static int in_subnet(struct in_addr netaddr, struct in_addr netmask, struct in_addr ip)
{
	app_trace (TRACE_INFO, "%lx %lx %lx %lx", iptohl(ip), iptohl(netmask), iptohl(ip) & iptohl(netmask), iptohl(netaddr));
	return ( (iptohl(ip) & iptohl(netmask)) == iptohl(netaddr) );
}

/*-----------------------------------------------------------------------*/

static char *loc_ip2str (struct in_addr ip, int id)
{
	return ip2str (htonl (ip.s_addr), id);
}

static void clean_route (route_entry *route)
{
	if (route)
	{
		memset (route, 0, sizeof (route_entry));
		route->vidx = -1;
	} else {
		int i;
		net_if_t *p_netif = &net_if[0];
		for (i = 0; i < NET_IFACE_MAX_SMG; i++)
		{
			memset (&p_netif->iface[i].gw_route, 0, sizeof (route_entry));
			memset (&p_netif->iface[i].subnet_route, 0, sizeof (route_entry));
			p_netif->iface[i].gw_route.vidx = -1;
			p_netif->iface[i].subnet_route.vidx = -1;
		}

		for (i = 0; i < NETWORK_MAX_ROUTES; i++)
		{
			memset(&p_netif->route[i], 0, sizeof (route_entry));
			p_netif->route[i].vidx = -1;
		}
	}
}

/*-----------------------------------------------------------------------*/

void route_show_list(int intf)
{
	int i;

	net_if_t *p_netif = &net_if[intf];
	route_entry *p_route;

	char s_dest[100], s_gw[100], s_mask[100];

	for (i = 0; i < NETWORK_MAX_ROUTES; i++)
	{
		p_route = &p_netif->route[i];
		if (!strlen (p_route->dev_name))
			continue;

		strcpy(s_dest, loc_ip2str (p_route->dest, 0));
		strcpy(s_gw,   loc_ip2str (p_route->gw, 0));
		strcpy(s_mask, loc_ip2str (p_route->mask, 0));

		app_trace(TRACE_INFO, "# ROUTE: %s: dest: %s, gw: %s, mask: %s, intf-idx %d",
						p_route->dev_name, s_dest, s_gw, s_mask, p_route->vidx);
	}

	struct iface_t *p_iface;

	for (i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		p_iface = &p_netif->iface[i];
		if ((p_iface->subnet_route.vidx < 0) || (p_iface->subnet_route.vidx < 0))
			continue;

		strcpy(s_dest, loc_ip2str (p_iface->subnet_route.dest, 0));
		strcpy(s_gw,   loc_ip2str (p_iface->subnet_route.gw, 0));
		strcpy(s_mask, loc_ip2str (p_iface->subnet_route.mask, 0));

		app_trace(TRACE_INFO, "# ROUTE: %s: dest: %s, gw: %s, mask: %s, intf-idx %d",
						p_iface->subnet_route.dev_name, s_dest, s_gw, s_mask, p_iface->subnet_route.vidx);
	}

	for (i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		p_iface = &p_netif->iface[i];
		if ((p_iface->gw_route.vidx < 0) || (p_iface->gw_route.vidx < 0))
			continue;

		strcpy(s_dest, loc_ip2str (p_iface->gw_route.dest, 0));
		strcpy(s_gw,   loc_ip2str (p_iface->gw_route.gw, 0));
		strcpy(s_mask, loc_ip2str (p_iface->gw_route.mask, 0));

		app_trace(TRACE_INFO, "# ROUTE: %s: dest: %s, gw: %s, mask: %s, intf-idx %d",
						p_iface->gw_route.dev_name, s_dest, s_gw, s_mask, p_iface->gw_route.vidx);
	}
}

/*-----------------------------------------------------------------------*/

route_entry *route_search(int intf, int iface_idx, struct in_addr ipaddr)
{
	net_if_t *p_netif = &net_if[intf];
	route_entry *p_route;
	int i;

	if(TraceNet() > 40)
	{
		app_trace(TRACE_INFO, "# ROUTE LIST: search route for ipaddr [%s] iface[%d]", loc_ip2str(ipaddr, 0), intf);

		route_show_list(intf);
	}

	if(TraceNet() > 70)
	{
		app_trace(TRACE_INFO, "# ROUTE: check static configured routes");
	}

	for (i = 0; i < NETWORK_MAX_ROUTES; i++)
	{
		/*static routes*/
		p_route = &p_netif->route[i];
		if (p_route->vidx < 0)
			continue;

		if(p_route->vidx != iface_idx)
		{
			if(TraceNet() > 70)
			{
				app_trace(TRACE_INFO, "# ROUTE: SKIP. dest: %s, gw: %s, mask: %s. intf-idx %d (other iface)",
				          loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}
			continue;
		}

		if(in_subnet(p_route->dest, p_route->mask, ipaddr))
		{
			if(TraceNet() > 40)
			{
				app_trace(TRACE_INFO, "# ROUTE LIST: found route entry");

				app_trace(TRACE_INFO, "# ROUTE: %s. dest: %s, gw: %s, mask: %s. intf-idx %d",
						p_route->dev_name, loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}

			return p_route;	/* found route entry */
		}else{
			if(TraceNet() > 70)
			{
				app_trace(TRACE_INFO, "# ROUTE: SKIP. dest: %s, gw: %s, mask: %s. intf-idx %d (other subnet)",
				          loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}
		}
	}

	if(TraceNet() > 70)
	{
		app_trace(TRACE_INFO, "# ROUTE: check basic netiface routes");
	}

	for (i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		/*default route. like a x.x.x.0/24*/
		p_route = &p_netif->iface[i].subnet_route;
		if (p_route->vidx < 0)
			continue;

		if(p_route->vidx != iface_idx)
		{
			if(TraceNet() > 70)
			{
				app_trace(TRACE_INFO, "# ROUTE: SKIP. dest: %s, gw: %s, mask: %s. intf-idx %d (other iface)",
				          loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}
			/*default route for other interface*/
			continue;
		}

		if(in_subnet(p_route->dest, p_route->mask, ipaddr))
		{
			if(TraceNet() > 40)
			{
				app_trace(TRACE_INFO, "# ROUTE LIST: found basic route.");

				app_trace(TRACE_INFO, "# ROUTE: %s. dest: %s, gw: %s, mask: %s. intf-idx %d",
							p_route->dev_name, loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}

			return p_route;				/* default route */
		}else{
			if(TraceNet() > 70)
			{
				app_trace(TRACE_INFO, "# ROUTE: SKIP. dest: %s, gw: %s, mask: %s. intf-idx %d (other subnet)",
				          loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}
		}
	}

	if(TraceNet() > 70)
	{
		app_trace(TRACE_INFO, "# ROUTE: check default routes");
	}

	for (i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		/*default gateway for current iface*/
		p_route = &p_netif->iface[i].gw_route;
		if (p_route->vidx < 0)
			continue;

		if(p_route->vidx != iface_idx)
		{
			if(TraceNet() > 70)
			{
				app_trace(TRACE_INFO, "# ROUTE: SKIP. dest: %s, gw: %s, mask: %s. intf-idx %d (other iface)",
				          loc_ip2str(p_route->dest, 0), loc_ip2str(p_route->gw, 1), loc_ip2str(p_route->mask, 2), p_route->vidx);
			}
			/*default route for other interface*/
			continue;
		}

		if(TraceNet() > 40)
		{
			app_trace(TRACE_INFO, "# ROUTE LIST: found default gateway for iface [%d].", iface_idx);

			app_trace(TRACE_INFO, "# ROUTE: %s. gw: %s. intf-idx %d",
						p_route->dev_name, loc_ip2str(p_route->gw, 0), p_route->vidx);
		}

		return p_route;
	}


	if(TraceNet() > 40)
	{
		app_trace(TRACE_INFO, "# ROUTE LIST: no route found");
	}

	return NULL;
}

/*-----------------------------------------------------------------------*/

#define ROUTE_CHECK_PERIOD 10

void net_tick(void)
{
	net_check(0);
}

/*-----------------------------------------------------------------------*/

static int if_get_ip(char *if_name, struct sockaddr_in *addr)
{
	struct ifconf       Ifc;
	struct ifreq        IfcBuf[MAX_NUM_IFREQ];
	struct ifreq        *pIfr;
	int num_ifreq;
	int i;
	int fd;

	Ifc.ifc_len = sizeof(IfcBuf);
	Ifc.ifc_buf = (char *) IfcBuf;

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		app_trace(TRACE_ERR, "net: get local ip: error getip::socket() for %s (errno %s)",  if_name, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SIOCGIFCONF, &Ifc) < 0)
	{
		app_trace(TRACE_ERR, "net: get local ip: error getip::ioctl SIOCGIFCONF for %s (errno %s)",  if_name, strerror(errno));
		close(fd);
		return -1;
	}

	num_ifreq = Ifc.ifc_len / sizeof(struct ifreq);
	for(pIfr=Ifc.ifc_req, i=0; i<num_ifreq; pIfr++, i++)
	{
		if(pIfr->ifr_addr.sa_family != AF_INET) continue;
		if(strcmp(if_name, pIfr->ifr_name)) continue;

		if(ioctl(fd, SIOCGIFADDR, pIfr)<0)
		{
			app_trace(TRACE_ERR, "net: get local ip: error getip::ioctl SIOCGIFADDR for %s", if_name);
			close(fd);
			return -1;
		}

		memcpy(addr, &(pIfr->ifr_addr), sizeof(struct sockaddr_in));
		close(fd);
		return 0;
	}

	close(fd);
	return -1;
}

/*-----------------------------------------------------------------------*/

static int if_get_mac(char *if_name, unsigned char * mac_id)
{
	struct ifreq        ifr;
	int fd;

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		app_trace(TRACE_ERR, "net: get local mac: error getmac::socket() for %s (errno %s)",  if_name, strerror(errno));
		return -1;
	}

	strcpy(ifr.ifr_name, if_name);

	if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
	{
		close(fd);
		return -2;
	}

	if(mac_id) memcpy(mac_id, (unsigned char *)&(ifr.ifr_hwaddr.sa_data), sizeof(struct sockaddr));

	close(fd);
	return 0;

}

/*-----------------------------------------------------------------------*/

static int if_check_existance(char *if_name)
{
	return if_get_mac(if_name, NULL);
}

/*-----------------------------------------------------------------------*/

void route_show(int idx)
{
	if(idx < 0 || idx >= MAX_IF) return;

	route_show_list(idx);
}

/*-----------------------------------------------------------------------*/

static int ipaddr_is_local(net_if_t *p_netif, struct in_addr ipaddr, int *vidx)
{
	int i;

	if(TraceNet() > 48)
	{
		app_trace(TRACE_INFO, "net: check whether IP <%s> is local.", loc_ip2str (ipaddr, 0));
	}

	for(i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		if(!p_netif->iface[i].ipaddr.sin_addr.s_addr) continue;

		if(TraceNet() > 48)
		{
			app_trace(TRACE_INFO, "net: interface <%s> IP addr <%s>.",
				p_netif->iface[i].if_name,
				loc_ip2str (p_netif->iface[i].ipaddr.sin_addr, 0));
		}

		if(!memcmp(&p_netif->iface[i].ipaddr.sin_addr, &ipaddr, sizeof(struct in_addr)))
		{
			*vidx = i;

			if(TraceNet() > 48)
			{
				app_trace(TRACE_INFO, "net: check whether IP <%s> is local. +", loc_ip2str (ipaddr, 0));
			}

			return 1;
		}
	}

	if(TraceNet() > 48)
	{
		app_trace(TRACE_INFO, "net: check whether IP <%s> is local. -", loc_ip2str (ipaddr, 0));
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

int net_check_iface(char *if_name, uint32_t *ipaddr)
{
	struct sockaddr_in addr = {0};

	int res = -1;

	if(ipaddr)
	{
		res = if_get_ip(if_name, &addr);

		*ipaddr = iptohl(addr.sin_addr);
	}else{
		res = if_check_existance(if_name);
	}

	return res;
}

/*-----------------------------------------------------------------------*/

int net_is_ipaddr_local(uint32_t ip, int *vidx)
{
	struct in_addr ipaddr;
	net_if_t *p_netif = &net_if[0];

	ipaddr.s_addr = htonl(ip);

	return ipaddr_is_local(p_netif, ipaddr, vidx);
}

/*-----------------------------------------------------------------------*/
/*search mac-address for RTP-stream creation*/
int if_mac_lookup(int idx, int iface_idx, struct in_addr ipaddr, unsigned char *mac_id, int *vidx)
{
	int                 s, ret;
	struct arpreq       areq;
	struct sockaddr_in *sin;
	route_entry   *entry;
	uint32_t gw_ip;
	char ipname[64];
	char ifname_str[64];

	if(idx < 0 || idx >= MAX_IF) return -1;

	net_if_t *p_netif = &net_if[idx];

	strcpy(ipname, loc_ip2str(ipaddr, 0));
	strcpy(ifname_str, idx ? NET_INTERFACE_LOCAL:NET_INTERFACE_DEFAULT);

	*vidx = -1;

	if(TraceNet())
	{
		app_trace(TRACE_INFO, "arp: mac lookup for ip <%s>", ipname);
	}

	if(ipaddr_is_local(p_netif, ipaddr, vidx))
	{
		if(TraceNet())
			app_trace(TRACE_INFO, "arp: <%s> is local address", ipname);

		memcpy(mac_id, p_netif->local_mac_buf, sizeof(areq.arp_ha.sa_data));
		return 0;
	}

	if ((entry=route_search(idx, iface_idx, ipaddr)) == NULL)
	{
		if(TraceNet())
			app_trace(TRACE_ERR, "arp: route not found for <%s>", ipname);

		gw_ip = config_get_default_gw();

		if(TraceNet())
			app_trace(TRACE_INFO, "arp: default gw from config <%s>", ip2str(gw_ip, 0));

		if(!gw_ip)
			return 2;
	}

	if(iptohl(entry->gw))
	{
		if(TraceNet())
		{
			app_trace(TRACE_INFO, "arp: found %s route <%s> for <%s> dev '%s'",
				!iptohl(entry->dest)?"default ":"", loc_ip2str(entry->gw, 0), ipname, entry->dev_name);
		}
		ipaddr = entry->gw;
	}
	else
	{
		if(TraceNet())
		{
			app_trace(TRACE_INFO, "arp: <%s> is local network dev '%s'", ipname, entry->dev_name);
		}
	}

	strcpy(ifname_str, entry->dev_name);
	*vidx = entry->vidx;

	/* Get an internet domain socket. */
	if((s=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		app_trace(TRACE_ERR, "arp: error mac_lookup::socket() (errno %s)", strerror(errno));
		return 3;
	}

	/* Make the ARP request. */
	memset(&areq, 0, sizeof(areq));

	sin = (struct sockaddr_in *) &areq.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr = ipaddr;

	sin = (struct sockaddr_in *) &areq.arp_ha;
	sin->sin_family = ARPHRD_ETHER;

	strncpy(areq.arp_dev, ifname_str, 15);

	errno = 0;

	if(ioctl(s, SIOCGARP, (caddr_t) &areq) == -1)
	{
		if(TraceNet())
			app_trace(TRACE_ERR, "arp: %s: error arp request %s <%s>",
					ifname_str, loc_ip2str(ipaddr, 0), strerror(errno));
		ret = -1;
	}else
	if(!(areq.arp_flags & ATF_COM))
	{
		if(TraceNet())
			app_trace(TRACE_ERR, "arp: %s: error arp request %s - incomplete",
					ifname_str, loc_ip2str(ipaddr, 0));
		ret = -1;
	}else{
		memcpy(mac_id, (unsigned char *)(&areq.arp_ha.sa_data), sizeof(areq.arp_ha.sa_data));

		if(TraceNet())
			app_trace(TRACE_INFO, "arp: %s: arp request %s - mac %02x:%02x:%02x:%02x:%02x:%02x",
					ifname_str, loc_ip2str(ipaddr, 0),
				mac_id[0], mac_id[1], mac_id[2], mac_id[3], mac_id[4], mac_id[5]);

		ret = 0;
	}

	close(s);
	return ret;
}

/*-----------------------------------------------------------------------*/

static void net_iface_ip_change_indication(int idx)
{
	if(!config_get_status())
	{
		return;
	}

	struct iface_t *p_iface_ext = &net_if[0].iface[idx];

	if(TraceNet() > 20)
	{
		app_trace(TRACE_INFO, "net: process IP-changed indication for iface [%d] '%s'", idx, p_iface_ext->if_name);
		app_trace(TRACE_INFO, "net: iface [%d] '%s' has %s%s%s%s",
			idx, p_iface_ext->if_name,
			config_IsRTPiface(idx) ? "RTP ":"", config_IsSIGiface(idx) ? "SIP ":"",
			config_IsRADIUSiface(idx) ? "RADIUS ":"", config_IsH323iface(idx) ? "H323":"");
	}

	if(!idx || config_IsRTPiface(idx))
	{
		msp_update_ip();
		mspcUpdateRTPsw(1); /*for setting ports range*/
		mspcUpdateRTPswIface();
	}

	if(config_IsSIGiface(idx))    sipt_add_cmd(SC_SendUpdateIP,  idx, 0);
	if(config_IsRADIUSiface(idx)) sipt_add_cmd(SC_SendRadiusCfg, idx, 0);
	if(config_IsH323iface(idx))   h323_proc_sendConfig(1);
}


/*-----------------------------------------------------------------------*/

void net_iface_show(void)
{
	net_if_t *p_netif = &net_if[0];
	struct iface_t *p_iface = NULL;
	int i;

	for(i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		p_iface = &p_netif->iface[i];

		if(strlen(p_iface->if_name))
		{
			app_trace(TRACE_INFO, "net: if[%d] name: %s. local IP: %s", i, p_iface->if_name, p_iface->ipaddr_str);
		}
	}
}

/*-----------------------------------------------------------------------*/

int net_check(int idx)
{
	if(idx < 0 || idx >= MAX_IF) return -1;

	if(idx) return 0; /*don't need*/

	int i, change = 0;

	net_if_t *p_netif = &net_if[idx];
	net_if_t netif;
	struct iface_t *p_iface_loc = NULL;
	struct iface_t *p_iface_ext = NULL;

	memset(&netif, 0, sizeof(netif));

	if(TraceNet() > 99)
	{
		app_trace(TRACE_INFO, "net: tick for net-ifaces");
	}

	for(i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		p_iface_ext = &p_netif->iface[i];
		p_iface_loc = &netif.iface[i];

		if(strlen(p_iface_ext->if_name))
		{
			strcpy(p_iface_loc->if_name, p_iface_ext->if_name);

			if(if_get_ip(p_iface_loc->if_name, &p_iface_loc->ipaddr))
			{
				memset(&p_iface_loc->ipaddr, 0, sizeof(p_iface_loc->ipaddr));
			}

			strcpy(p_iface_loc->ipaddr_str, inet_ntoa(p_iface_loc->ipaddr.sin_addr));

			if(TraceNet() > 80)
			{
				app_trace(TRACE_INFO, "net: tick for if <%s>. current IP <%s>, prev IP <%s>",
						p_iface_loc->if_name, p_iface_loc->ipaddr_str, p_iface_ext->ipaddr_str);
			}
		}
	}

	for(i = 0; i < NET_IFACE_MAX_SMG; i++)
	{
		p_iface_ext = &p_netif->iface[i];
		p_iface_loc = &netif.iface[i];

		if(strlen(p_iface_ext->if_name))
		{
			if(memcmp(&p_iface_loc->ipaddr.sin_addr, &p_iface_ext->ipaddr.sin_addr, sizeof(p_iface_loc->ipaddr.sin_addr)))
			{
				if(TraceNet() > 20)
				{
					app_trace(TRACE_INFO, "net: tick for if <%s>. current IP <%s>, prev IP <%s>. IP-changed",
										p_iface_loc->if_name, p_iface_loc->ipaddr_str, p_iface_ext->ipaddr_str);
				}

				memcpy(&p_iface_ext->ipaddr, &p_iface_loc->ipaddr, sizeof(p_iface_loc->ipaddr));
				strcpy(p_iface_ext->ipaddr_str, inet_ntoa(p_iface_ext->ipaddr.sin_addr));

				net_iface_ip_change_indication(i);

				change++;
			}
		}
	}

	// if(change) route_init(idx);

	if(TraceNet() > 99)
	{
		app_trace(TRACE_INFO, "--------------------------------------------");
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

int net_init_iface(int id, int vid, int alias)
{
	if(id < 0 || id >= NET_IFACE_MAX_SMG) return -1;

	net_if_t *p_netif = &net_if[0];
	struct iface_t *p_iface = &p_netif->iface[id];
	char alias_str[32] = {0};

	if(alias)
	{
		sprintf(alias_str, ":%d", alias);
	}

	if(vid)
	{
		sprintf(p_iface->if_name, "%s.%d%s", NET_INTERFACE_BASE, vid, alias_str);
	}else{
		sprintf(p_iface->if_name, "%s%s", NET_INTERFACE_DEFAULT, alias_str);
	}

	if(if_get_ip(p_iface->if_name, &p_iface->ipaddr))
	{
		app_trace(-1, "net: init: get IP address failed for '%s'", p_iface->if_name);
		return -3;
	}

	strcpy(p_iface->ipaddr_str, inet_ntoa(p_iface->ipaddr.sin_addr));

	if(TraceNet())
	{
		app_trace(TRACE_INFO, "net: init: if name: %s. id %d", p_iface->if_name, id);
		app_trace(TRACE_INFO, "net: init: local IP: %s", p_iface->ipaddr_str);
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

int net_close_route (int idx)
{
	if (idx < 0 || idx >= NETWORK_MAX_ROUTES)
		return -1;

	net_if_t *p_netif = &net_if[0];

	clean_route (&p_netif->route[idx]);

	return 0;
}

int net_init_route (int idx)
{
	if (idx < 0 || idx >= NETWORK_MAX_ROUTES)
		return -1;

	int i;
	route_entry *p_in_route;
	net_if_t *p_netif = &net_if[0];
	stNetRoute *p_route = (stNetRoute *)config_object_ptr (CFGO_NETWORK_ROUTES, idx);

	if (!p_route)
		return -1;

	net_close_route (idx);

	p_in_route = &p_netif->route[idx];

	if (p_route->ifaceID == 0)
	{
		int iface_count = config_obj_count(CFGO_NET_IFACE);

		for (i = 0; i < iface_count; i++)
		{
			struct in_addr mask;
			struct in_addr ip;
			struct in_addr gw;
			stNetIface *p_iface = (stNetIface *)config_object_ptr (CFGO_NET_IFACE, i);
			if (!p_iface)
				continue;

			if (p_iface->type == eNET_IFACE_TAGGED)
			{
				ip.s_addr = htonl (p_iface->iface.tagged.ipv4.addr);
				mask.s_addr = htonl (p_iface->iface.tagged.ipv4.netmask);
			} else
			if (p_iface->type == eNET_IFACE_UNTAGGED)
			{
				ip.s_addr = htonl (p_iface->iface.untagged.ipv4.addr);
				mask.s_addr = htonl (p_iface->iface.untagged.ipv4.netmask);
			} else {
				continue;
			}

			gw.s_addr = htonl (p_route->gateway);

			if ((ip.s_addr & mask.s_addr) == (gw.s_addr & mask.s_addr))
			{
				p_route->ifaceID = p_iface->ID;
				p_in_route->vidx = i;
				break;
			}
		}

		if (!p_route->ifaceID)
			return -1;
	}

	p_in_route->dest.s_addr = htonl (p_route->target);
	p_in_route->gw.s_addr = htonl (p_route->gateway);
	p_in_route->mask.s_addr = htonl (p_route->net_mask);

	char dev_name[64];

	if (config_get_net_iface_name_by_id (p_route->ifaceID, dev_name) < 0)
	{
		app_trace (TRACE_ERR, "net: failed to add default routes for iface idx - %d", idx);
		return -1;
	}

	int name_len = strlen(dev_name);
	for (i = 0; i < name_len; i++)
	{
		if (dev_name[i] == ':')
			dev_name[i] = '\0';
	}

	strcpy (p_in_route->dev_name, dev_name);

	return 0;
}

int net_update_route (int idx)
{
	if (idx < 0 || idx >= NETWORK_MAX_ROUTES)
		return -1;

	net_if_t *p_netif = &net_if[0];

	stNetIface *p_iface = (stNetIface *)config_object_ptr (CFGO_NET_IFACE, idx);

	if (!p_iface)
		return -1;

	if (p_iface->type == eNET_IFACE_TAGGED)
	{
		if (p_iface->iface.tagged.ipv4.gateway)
		{
			p_netif->iface[idx].gw_route.dest.s_addr = 0; /*default*/
			p_netif->iface[idx].gw_route.gw.s_addr = htonl (p_iface->iface.tagged.ipv4.gateway);
		}

		p_netif->iface[idx].subnet_route.dest.s_addr = htonl (p_iface->iface.tagged.ipv4.addr) & htonl (p_iface->iface.tagged.ipv4.netmask);
		//p_netif->iface[idx].subnet_route.gw.s_addr = htonl (p_iface->iface.tagged.ipv4.gateway);
		p_netif->iface[idx].subnet_route.mask.s_addr = htonl (p_iface->iface.tagged.ipv4.netmask);
	} else
	if (p_iface->type == eNET_IFACE_UNTAGGED)
	{
		if (p_iface->iface.untagged.ipv4.gateway)
		{
			p_netif->iface[idx].gw_route.dest.s_addr = 0; /*default*/
			p_netif->iface[idx].gw_route.gw.s_addr = htonl (p_iface->iface.untagged.ipv4.gateway);
		}

		p_netif->iface[idx].subnet_route.dest.s_addr = htonl (p_iface->iface.untagged.ipv4.addr) & htonl (p_iface->iface.untagged.ipv4.netmask);
		//p_netif->iface[idx].subnet_route.gw.s_addr = htonl (p_iface->iface.untagged.ipv4.gateway);
		p_netif->iface[idx].subnet_route.mask.s_addr = htonl (p_iface->iface.untagged.ipv4.netmask);
	} else {
		return 0;
	}

	if (p_netif->iface[idx].gw_route.gw.s_addr)
		p_netif->iface[idx].gw_route.vidx = idx;

	p_netif->iface[idx].subnet_route.vidx = idx;

	char dev_name[64];

	if (config_get_net_iface_name_by_idx (idx, dev_name) < 0)
	{
		app_trace (TRACE_ERR, "net: failed to add default routes for iface idx - %d", idx);
		return -1;
	}

	int i;
	int name_len = strlen(dev_name);
	for (i = 0; i < name_len; i++)
	{
		if (dev_name[i] == ':')
			dev_name[i] = '\0';
	}

	strcpy (p_netif->iface[idx].gw_route.dev_name, dev_name);
	strcpy (p_netif->iface[idx].subnet_route.dev_name, dev_name);

	return 0;
}

int net_close_iface(int idx)
{
	if(idx < 0 || idx > NET_IFACE_MAX_SMG) return -1;

	net_if_t *p_netif = &net_if[0];

	p_netif->iface[idx].if_name[0] = '\0';
	p_netif->iface[idx].ipaddr_str[0] = '\0';

	memset(&p_netif->iface[idx], 0, sizeof(p_netif->iface[idx]));

	return 0;
}

/*-----------------------------------------------------------------------*/

int net_init(int idx)
{
	if(idx < 0 || idx >= MAX_IF) return -1;

	net_if_t *p_netif = &net_if[idx];

	memset(p_netif, 0, sizeof(*p_netif));
	clean_route (NULL); /*clean all routes*/
	sprintf(p_netif->iface[0].if_name, idx ? NET_INTERFACE_LOCAL:NET_INTERFACE_DEFAULT);

	p_netif->iface[0].ipaddr_str[0] = 0;

	int res = 0;
	int attempts = 10;

	do
	{
		res = if_get_ip(p_netif->iface[0].if_name, &p_netif->iface[0].ipaddr);

		if(res)
		{
			memset(&p_netif->iface[0].ipaddr.sin_addr, 0, sizeof(struct sockaddr_in));
			res = 0;
		}
	}while(res && (attempts-- > 0));

	if(res) return -1;

	int mac_res = if_get_mac(p_netif->iface[0].if_name, p_netif->local_mac_buf);

	if(mac_res)
	{
		app_trace(-1, "net: init: get mac address failed for '%s'", p_netif->iface[0].if_name);
		return -2;
	}


	strcpy(p_netif->iface[0].ipaddr_str, inet_ntoa(p_netif->iface[0].ipaddr.sin_addr));

	/*if(TraceNet())*/
	{
		app_trace(TRACE_INFO, "net: init: if name: %s. idx %d", p_netif->iface[0].if_name, idx);
		app_trace(TRACE_INFO, "net: init: local IP: %s", p_netif->iface[0].ipaddr_str);
		app_trace(TRACE_INFO, "net: init: local MAC: %02x:%02x:%02x:%02x:%02x:%02x",
				p_netif->local_mac_buf[0]&0xFF, p_netif->local_mac_buf[1]&0xFF, p_netif->local_mac_buf[2]&0xFF,
				p_netif->local_mac_buf[3]&0xFF, p_netif->local_mac_buf[4]&0xFF, p_netif->local_mac_buf[5]&0xFF);
	}

	if(!idx && !net_thread)
	{
		net_thread = (ut_thread_t*)ut_calloc(1,sizeof(ut_thread_t));
		ut_thread_create(UT_THREAD_STACK_SIZE, net_thread, net_task, NULL);
	}

	return 0;
}

/*-----------------------------------------------------------------------*/

void net_close(int idx)
{
	net_running = 0;

	if(NULL != net_thread)
	{
		ut_thread_cancel(*net_thread);
		ut_thread_join(net_thread);
		ut_free_ex(net_thread);
		net_thread = NULL;
	}
}

/*-----------------------------------------------------------------------*/

char *net_ip(int idx)
{
	if(idx < 0 || idx >= MAX_IF) return NULL;

	return net_if[idx].iface[0].ipaddr_str;
}

/*-----------------------------------------------------------------------*/

struct in_addr net_addr(int idx)
{
	struct in_addr s_addr;
	memset(&s_addr, 0, sizeof(s_addr));
	if(idx < 0 || idx >= MAX_IF) return s_addr;

	return net_if[idx].iface[0].ipaddr.sin_addr;
}

/*-----------------------------------------------------------------------*/

struct in_addr net_addr_by_name(const char *iface_name)
{
	int i, idx;
	struct in_addr s_addr;
	memset(&s_addr, 0, sizeof(s_addr));
	if(!iface_name || !strlen(iface_name))
		return s_addr;

	for(idx = 0; idx < MAX_IF; ++idx)
		for(i = 0; i < NET_IFACE_MAX_SMG; ++i)
			if(!strcmp(net_if[idx].iface[i].if_name, iface_name))
				return net_if[idx].iface[i].ipaddr.sin_addr;

	return s_addr;
}

/*-----------------------------------------------------------------------*/

struct in_addr net_iface_addr(int idx)
{
	struct in_addr s_addr;
	memset(&s_addr, 0, sizeof(s_addr));
	if(idx < 0 || idx >= NET_IFACE_MAX_SMG) return s_addr;

	return net_if[0].iface[idx].ipaddr.sin_addr;
}

/*-----------------------------------------------------------------------*/

int net_mac(int idx, unsigned char *mac)
{
	if(idx < 0 || idx >= MAX_IF) return -1;

	memcpy(mac, net_if[idx].local_mac_buf, 6);

	return 0;
}


/*-----------------------------------------------------------------------*/

enum {
	DEFDATALEN = 56,
	MAXIPLEN = 60,
	MAXICMPLEN = 76,
	MAXPACKET = 65468,
	MAX_DUP_CHK = (8 * 128),
	MAXWAIT = 10,
	PINGINTERVAL = 1		/* second */
};

/* common routines */
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

/*-----------------------------------------------------------------------*/

int ping(int iface_idx, struct in_addr ipaddr)
{
	int IP4_HDRLEN  = 20;
	int ICMP_HDRLEN = 8;

	int sd, ip_flags[4] = {0};
	const int on = 1;
	struct ip iphdr;
	struct icmp icmphdr;
	uint8_t packet[IP_MAXPACKET];
	struct sockaddr_in sin;
	struct ifreq ifr;
	char dev_name[16] = {0};
	int name_len = 0;
	int i;

	struct iface_t *p_iface = &net_if[0].iface[iface_idx];

	// Submit request for a socket descriptor to look up interface.
	if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		app_trace (TRACE_WARN, "socket() failed to get socket descriptor for using ioctl() ");
		return 1;
	}

	memset (&ifr, 0, sizeof (ifr));

	name_len = strlen(p_iface->if_name);
	for (i = 0; i < name_len; i++)
	{
		if (p_iface->if_name[i] == ':')
			break;

		dev_name[i] = p_iface->if_name[i];
	}

	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", dev_name);
	if (ioctl (sd, SIOCGIFINDEX, &ifr) < 0)
	{
		app_trace (TRACE_WARN, "ioctl() failed to find interface ");
		return 1;
	}
	close (sd);

	//IPv4 header
	iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);
	iphdr.ip_v = 4;
	iphdr.ip_tos = 0;
	iphdr.ip_len = htons (IP4_HDRLEN + ICMP_HDRLEN);
	iphdr.ip_id = htons (0);

	memset (ip_flags, 0, sizeof (ip_flags));

	ip_flags[1] = 1;

	iphdr.ip_off = htons ((ip_flags[0] << 15)
	                    + (ip_flags[1] << 14)
	                    + (ip_flags[2] << 13)
	                    +  ip_flags[3]);

	iphdr.ip_ttl = 64;
	iphdr.ip_p = IPPROTO_ICMP;

	inet_pton (AF_INET, p_iface->ipaddr_str, &(iphdr.ip_src));
	memcpy(&iphdr.ip_dst, &ipaddr, sizeof(iphdr.ip_dst));

	iphdr.ip_sum = 0;
	iphdr.ip_sum = in_cksum ((uint16_t *) &iphdr, IP4_HDRLEN);

	// ICMP header
	icmphdr.icmp_type = ICMP_ECHO;
	icmphdr.icmp_code = 0;
	icmphdr.icmp_id = htons (1000);
	icmphdr.icmp_seq = htons (0);
	icmphdr.icmp_cksum = 0;

	// Prepare packet.
	memcpy (packet, &iphdr, IP4_HDRLEN);
	memcpy ((packet + IP4_HDRLEN), &icmphdr, ICMP_HDRLEN);

	icmphdr.icmp_cksum = in_cksum ((uint16_t *) (packet + IP4_HDRLEN), ICMP_HDRLEN);
	memcpy ((packet + IP4_HDRLEN), &icmphdr, ICMP_HDRLEN);

	memset (&sin, 0, sizeof (struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = iphdr.ip_dst.s_addr;

	if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		app_trace (TRACE_ERR, "socket() failed, cause: %s", strerror(errno));
		return 1;
	}

	if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0)
	{
		app_trace (TRACE_ERR, "setsockopt() failed to set IP_HDRINCL, cause: %s", strerror(errno));
		return 1;
	}

	if (setsockopt (sd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0)
	{
		app_trace (TRACE_ERR, "setsockopt() failed to bind to interface, cause: %s", strerror(errno));
		return 1;
	}

	if (sendto (sd, packet, IP4_HDRLEN + ICMP_HDRLEN, 0, (struct sockaddr *) &sin, sizeof (struct sockaddr)) < 0)
	{
		app_trace (TRACE_ERR, "sendto() failed, cause: %s", strerror(errno));
		return 1;
	}

	close (sd);
	return 0;
}

/*-----------------------------------------------------------------------*/
void sipt_InterfaceResolveCheck();
void h323_InterfaceResolveCheck();

static void *net_task(void *ptr)
{
	ut_thread_setname_np("mgapp-network");

	app_trace(TRACE_INFO, "net task: start. pid %i", get_ptid());

#ifdef STORM_TEST_THREAD
	app_trace(TRACE_INFO, "net task: pause for %d sec", tp_Net*30);
	app_pause(tp_Net*TEST_THREAD_DELAY);
	app_trace(TRACE_INFO, "net task: continue process");
#endif

/*
	char proc_str[64];
	sprintf(proc_str, "[%i] net task", get_ptid());
	add_proc_info(proc_str);

	app_bk_pid(get_ptid(), PID_TYPE_CONFIG, 0, 0);
*/
	net_running = 1;

	while(net_running)
	{
		if(TraceNet() > 50)
		{
			app_trace(TRACE_INFO, "net task: cfg-status [%d]", config_get_status());
		}

		if(config_get_status())
		{
			app_pause_sec(1);

			net_tick();

			sipt_InterfaceResolveCheck();
			/*h323_InterfaceResolveCheck();*/
		}else{
			app_pause(100);
		}
	}

	app_trace(TRACE_INFO, "net task: exit");

	return NULL;
}

/*-----------------------------------------------------------------------*/

char *net2str(struct in_addr ipaddr, char *s, int ssize)
{
	if(!s) return "";
	s[0] = 0;
	inet_ntop(AF_INET, &ipaddr, s, ssize);

	if(!strcmp(s, "0.0.0.0")) s[0] = 0;
	return s;
}

/*-----------------------------------------------------------------------*/


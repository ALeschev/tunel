#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/icmp.h>

#include "statistics.c"

static unsigned int rtpsw_main_hook(unsigned int hooknum,
                                    struct sk_buff *skb,
                                    const struct net_device *in,
                                    const struct net_device *out,
                                    int (*okfn)(struct sk_buff*))
{
	struct iphdr *p_iphdr;
	struct udphdr *p_udphdr;

	st_pkt_in_inc();

	p_iphdr = (struct iphdr *)(skb->network_header);
	p_udphdr = (struct udphdr *)(skb->data + p_iphdr->ihl * 4);

	if (!p_iphdr || !p_udphdr)
	{
		st_pkt_drop_inc();
		return NF_ACCEPT;
	}

	rtpsw_info("%08x:%u -> %08x:%u device '%s' sock <%p>",
	           ntohl(p_iphdr->saddr), ntohs(p_udphdr->source),
	           ntohl(p_iphdr->daddr), ntohs(p_udphdr->dest),
	           skb->dev? skb->dev->name:"none",
	           skb->sk);

	st_pkt_out_inc();

	return NF_ACCEPT;
}

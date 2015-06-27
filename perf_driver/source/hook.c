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
	struct iphdr *iph;
	// struct udphdr *udph;
	// struct icmphdr *icmph;

	if (skb->protocol != htons(ETH_P_IP))
	{
		return NF_ACCEPT;
	}

	iph = ip_hdr(skb);

	rtpsw_info("catch: 0x%08X -> 0x%08X", iph->saddr, iph->daddr);

	return NF_ACCEPT;
}
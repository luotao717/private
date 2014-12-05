
#define MAX_NETSPY_ENTRY 1024
#include <linux/types.h>

typedef struct netspy_fctl_ipport_entry
{
        __be32 netspy_sip;
	__be16 netspy_sport;
	__be32 netspy_dip;
	__be16 netspy_dport;
	__u8 netspy_protocol;
	__be32 netspy_seqnum;
}T_IPPORT_FCTL_ENTRY;

typedef struct netspy_ipport_fctl_
{
	unsigned int is_Enable;
	unsigned int downSwitchRx;
	unsigned int downSwitchTx;
	T_IPPORT_FCTL_ENTRY lan_ipport_entry_tbl[MAX_NETSPY_ENTRY];
	//T_MAC_FCTL_ENTRY lan_other_ip_entry;
}T_NETSPY_IPPORT_FCTL_, *T_NETSPY_IPPORT_FCTL_p;

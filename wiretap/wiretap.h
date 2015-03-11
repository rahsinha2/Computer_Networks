#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include "/usr/include/netinet/ether.h"
#include "/usr/include/netinet/ip.h"
#include "/usr/include/netinet/tcp.h"
#include "/usr/include/netinet/udp.h"
#include "/usr/include/arpa/inet.h"
#include "/usr/include/linux/if_ether.h"
#include "/usr/include/linux/icmp.h"
#include "/usr/include/pcap/bpf.h"
#include <pcap/pcap.h>
#include <time.h>

#define FILE_NAME_MAX 1024
#define FIRST_PACKET_INDEX 1
#define OCTETS_IN_IP 4
#define BYTE_LEN 8
#define IP_MASK 0xFF
#define BUFFER_SIZE 50
#define ROW 10000
#define COL 50
#define time_len 20
#define OCT 256
#define PROTO_ICMP 1
#define PROTO_TCP 6
#define PROTO_UDP 17
#define ICMP_CODE_ZERO 666
#define ZERO 0
#define IP_HDR_SIZE 20

typedef struct wt_data {
	char filename[FILE_NAME_MAX];

} wt_data;
struct arphdr_mod
  {
    unsigned short int ar_hrd;          /* Format of hardware address.  */
    unsigned short int ar_pro;          /* Format of protocol address.  */
    unsigned char ar_hln;               /* Length of hardware address.  */
    unsigned char ar_pln;               /* Length of protocol address.  */
    unsigned short int ar_op;           /* ARP opcode (command).  */

    unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
    unsigned char __ar_sip[4];          /* Sender IP address.  */
    unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
    unsigned char __ar_tip[4];          /* Target IP address.  */
  } arphdr_mod;

struct tcp_option_t {
  uint8_t kind;
  uint8_t size;
} tcp_option_t;

void dummy_func(int temp);
void parse_args(int argc, char** argv, wt_data* object);
void print_help();
void read_packet(u_char* , const struct pcap_pkthdr*, const u_char*);
void extract_packet(u_char* u, const struct pcap_pkthdr* pkt, const u_char* packet);
void print_count(unsigned char array [][COL], int count);
void print_count_int(int * array, int count);
void print_tcp_flags();
void print_tcp_options();
void print_count_ttl(int* array, int count);

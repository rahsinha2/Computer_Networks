#include "wiretap.h"
extern int t_0, t_1, t_2, t_3, t_4, t_5, t_8, t_10;
extern int fin, syn, rst, psh, ack, urg;
extern int count;
extern int count2;
extern unsigned int smallest;
extern unsigned int largest;
extern unsigned int average;
extern struct timeval ts1;
extern struct timeval ts2;
extern struct iphdr * ip;
extern struct ethhdr * eth;
extern struct arphdr_mod * arp;
extern struct tcphdr * tcp;
extern struct udphdr * udp ;
extern struct icmphdr * icmp;
extern struct tcp_option_t * tcp_opt;
extern char s_date[time_len];

extern unsigned char s_eth_add[ROW][COL];
extern unsigned char s_ip_add[ROW][COL];
extern unsigned char d_eth_add[ROW][COL];
extern unsigned char d_ip_add[ROW][COL];
extern unsigned char arp_table[ROW][COL];
extern int net_proto[ROW];
extern int trans_proto[ROW];
extern int ttl[ROW];
extern int s_udp[ROW];
extern int d_udp[ROW];
extern int s_tcp[ROW];
extern int d_tcp[ROW];
extern int icmp_code[ROW];
extern int icmp_type[ROW];
extern uint8_t* opt;
void dummy_func(int temp)
{
	printf("\nThis is  a dummy function   Temp = %d\n", temp);

}
void parse_args(int argc, char** argv, wt_data* object)
{
  
    int c = 0;
    memset(object->filename, 0, FILE_NAME_MAX);
    while (1) 
    {   
      static struct option long_options[] =
        {   
          {"help",    no_argument,0, 'a'},
          {"open",    required_argument, 0, 'b'},
          {0, 0, 0, 0}
        };  
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "ab:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1) 
        break;

      switch (c) 
        {   
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'a':
	  print_help();
	  exit(0);
          break;

       case 'b':
	  strncpy(object->filename, optarg, FILE_NAME_MAX);          
          break;

        default:
          printf("\nERROR: Invalid option\n");
	  print_help(); 
          exit(1);
        }
    }


	if (argc > 3)	{
		printf("\nERROR: Excess no. of arguments\n");
		print_help();
		exit(1);
	}

	if (strcmp("--help", object->filename) == 0)	{
		print_help();
		exit(1);
	}

}

void print_help()
{
	fprintf(stdout,
          "wiretap [OPTIONS] filename\n"
          "  --help        	\t Print this help screen\n"
          "  --open file_name 	\t Packet capture filename to open\n");
}
void print_count_ttl(int* array, int count)
{

int i=0;
int counter = 0;
int j = 0;
        for ( i = 0; i<count; i++) {
                counter = 1;
                for (j = i+1; j<= count-1; j++) {
                        if ((array[i] == array[j]) && array[i] != '\0') {
                                counter++;
                                array[j]= '\0';
                        }
                }

                if(array[i] != '\0') {

                                printf("%d\t\t\t%d\n", array[i], counter);


                }

        }

}
void print_tcp_options()
{
        if (t_0>0)      printf("0 (0x0)         %d\n", t_0);
        if (t_1>0)      printf("1 (0x1)         %d\n", t_1);
        if (t_2>0)      printf("2 (0x2)         %d\n", t_2);
        if (t_3>0)      printf("3 (0x3)         %d\n", t_3);
        if (t_4>0)      printf("4 (0x4)         %d\n", t_4);
        if (t_5>0)      printf("5 (0x5)         %d\n", t_5);
        if (t_8>0)      printf("8 (0x8)         %d\n", t_8);
        if (t_10>0)     printf("10(0xA)         %d\n", t_10);
}

void print_tcp_flags()
{
        printf("ACK             %d\n", ack);
        printf("FIN             %d\n", fin);
        printf("PSH             %d\n", psh);
        printf("RST             %d\n", rst);
        printf("SYN             %d\n", syn);
        printf("URG             %d\n", urg);
}

void print_count(unsigned char array[][COL], int count)
{

int i=0;
int counter = 0;
int j = 0;
        for ( i = 0; i<count; i++) {
                counter = 1;
                for (j = i+1; j<= count-1; j++) {
                        if ((strcmp((char*)array[i],(char*)array[j])== 0) && array[i][0] != '\0') {
                                counter++;
                                array[j][0] = '\0';
                        }
                }

                if(array[i][0] != '\0')
                        printf("%-40s           %d\n", array[i], counter);

        }


}

void print_count_int(int* array, int count)
{

int i=0;
int counter = 0;
int j = 0;
        for ( i = 0; i<count; i++) {
                counter = 1;
                for (j = i+1; j<= count-1; j++) {
                        if ((array[i] == array[j]) && array[i] != '\0') {
                                counter++;
                                array[j]= '\0';
                        }
                }
                
                if(array[i] != '\0') {
                        if (array[i] == ETH_P_ARP)
                        printf("%s\t\t\t%d\n", "ARP", counter);
                
                        else if (array[i] == ETH_P_IP)
                        printf("%s\t\t\t%d\n", "IP", counter);
                
                        else if (array[i] == PROTO_ICMP)
                        printf("%s\t\t\t%d\n", "ICMP", counter);
                        
                        else if (array[i] == PROTO_TCP)
                        printf("%s\t\t\t%d\n", "TCP", counter);
                        
                        else if (array[i] == PROTO_UDP)
                        printf("%s\t\t\t%d\n", "UDP", counter);
                        
                        else if (array[i] == ICMP_CODE_ZERO)
                        printf("%d\t\t\t%d\n", ZERO, counter);
                        
                        else
                        printf("%d\t\t\t%d\n", array[i], counter);
                }
        }       
}

void read_packet(u_char* u, const struct pcap_pkthdr* pkt, const u_char* packet)
{
        int size_ip = 0;    
        struct tm * time_c = NULL;
        ++count;
        if (pkt->len < smallest)        {   
                smallest = pkt->len;
        }    
        if (pkt->len > largest)         {   
                largest = pkt->len;
        }   
        average += pkt->len;    
    
        if(count == FIRST_PACKET_INDEX) 
                ts1 = pkt->ts;
        ts2 = pkt->ts;
        time_c = localtime(&ts1.tv_sec);
        strftime(s_date, time_len, "%Y-%m-%d %H:%M:%S %Z", time_c);
            
        eth = (struct ethhdr*)packet;
        ip = (struct iphdr*)(packet + ETHER_HDR_LEN);
        size_ip = ip->ihl*OCTETS_IN_IP;

        if (size_ip < IP_HDR_SIZE) {
                return;
        }   
        /* print source and destination IP addresses */
        int i = 0;
        unsigned char octet[OCTETS_IN_IP] = {0,0,0,0};
        unsigned char octet1[OCTETS_IN_IP] = {0,0,0,0};
        for (i=0; i<OCTETS_IN_IP; i++)
        {   
                octet[i] = ( ip->saddr >> (i*BYTE_LEN) ) & IP_MASK;
        }   
            
        for (i=0; i<OCTETS_IN_IP; i++)
        {   
                octet1[i] = ( ip->daddr >> (i*BYTE_LEN) ) & IP_MASK;
        }   

}

void extract_packet(u_char* u, const struct pcap_pkthdr* pkt, const u_char* packet)
{
        ++count2;
        int i = count2-1;
        int k = 0;
        int opt_len = 0;
        int len_read = 0;
        int flag = 0;
        unsigned char buffer1[BUFFER_SIZE];
        unsigned char buffer2[BUFFER_SIZE];
        unsigned char buffer3[BUFFER_SIZE];
        memset(buffer1, 0, BUFFER_SIZE);
        memset(buffer2, 0, BUFFER_SIZE);
        eth = (struct ethhdr*)packet;
        ip = (struct iphdr*)(packet + ETHER_HDR_LEN);
        arp = (struct arphdr_mod*)(packet + ETHER_HDR_LEN);

        if (ntohs(ip->protocol)/OCT == PROTO_TCP)       {
                tcp = (struct tcphdr*)(packet + ETHER_HDR_LEN + sizeof(struct iphdr));
                if (tcp->fin == 1)      fin++;
                if (tcp->ack == 1)      ack++;
                if (tcp->rst == 1)      rst++;
                if (tcp->psh == 1)      psh++;
                if (tcp->syn == 1)      syn++;
                if (tcp->urg == 1)      urg++;

                opt_len = (tcp->doff * OCTETS_IN_IP) - IP_HDR_SIZE;

                if (opt_len > ZERO)        {
                        opt = (uint8_t*)(packet + ETHER_HDR_LEN + sizeof(struct iphdr) + IP_HDR_SIZE);
                       while(len_read < opt_len) {
                                tcp_opt = (struct tcp_option_t*)opt;

                                if (tcp_opt->kind == TCPOPT_EOL) {
                                        t_0++;
                                        opt+=1;
                                        len_read+=1;
                                        continue;
                                }
                                else if (tcp_opt->kind == TCPOPT_NOP) {
                                        if (flag == ZERO)
                                                t_1++;
                                        flag =1;
                                        opt+=1;
                                        len_read+=1;
                                        continue;
                                }
                                else if (tcp_opt->kind == TCPOPT_MAXSEG || tcp_opt->kind == TCPOLEN_SACK_PERMITTED) {
                                        t_2++;
                                        opt += tcp_opt->size;
                                        len_read+= tcp_opt->size;
                                }
                                else if (tcp_opt->kind == TCPOLEN_MAXSEG || tcp_opt->kind == TCPOPT_SACK_PERMITTED) {
                                        t_4++;
                                        opt += tcp_opt->size;
                                        len_read+= tcp_opt->size;
                                }
                                else if (tcp_opt->kind == TCPOPT_WINDOW || tcp_opt->kind == TCPOLEN_WINDOW)     {
                                        t_3++;
                                        opt += tcp_opt->size;
                                        len_read+= tcp_opt->size;
                                }
                                else if (tcp_opt->kind == TCPOPT_SACK)  {
                                        t_5++;
                                        opt += tcp_opt->size;
                                        len_read+= tcp_opt->size;
                                }
                                else if (tcp_opt->kind == TCPOPT_TIMESTAMP)     {
                                        t_8++;
                                        opt += tcp_opt->size;
                                        len_read+= tcp_opt->size;
                                }

                                else if (tcp_opt->kind == TCPOLEN_TIMESTAMP)    {
                                        t_10++;
                                        opt += tcp_opt->size;
                                        len_read+= tcp_opt->size;
                                }
                        }
                }
        }
        else if (ntohs(ip->protocol)/OCT == PROTO_UDP)
                udp = (struct udphdr*)(packet + ETHER_HDR_LEN + sizeof(struct iphdr));
        else if (ntohs(ip->protocol)/OCT == PROTO_ICMP)
                icmp = (struct icmphdr*)(packet + ETHER_HDR_LEN + sizeof(struct iphdr));


        net_proto[i] = ntohs(eth->h_proto);

        if ( ntohs(eth->h_proto) == ETH_P_IP) {
                trans_proto[i] = ntohs(ip->protocol)/OCT;
                ttl[i] = ntohs(ip->ttl)/OCT;

                if ( trans_proto[i] == PROTO_TCP)       {
                        s_tcp[i] = ntohs(tcp->source);
                        d_tcp[i] = ntohs(tcp->dest);
                }
                else if( trans_proto[i] == PROTO_UDP)   {
                        s_udp[i] = ntohs(udp->source);
                        d_udp[i] = ntohs(udp->dest);
                }
                else if( trans_proto[i] == PROTO_ICMP)  {
                        icmp_type[i] = ntohs(icmp->type)/OCT;
                        if (ntohs(icmp->code)/OCT == ZERO)      icmp_code[i] = ICMP_CODE_ZERO;
                        else
                        icmp_code[i] = ntohs(icmp->code)/OCT;
                }
        }

        snprintf((char*)buffer1,BUFFER_SIZE,"%02x:%02x:%02x:%02x:%02x:%02x%s", eth->h_source[0], eth->h_source[1], eth->h_source[2],eth->h_source[3], eth->h_source[4], eth->h_source[5],"\0");
        snprintf((char*)buffer2,BUFFER_SIZE,"%02x:%02x:%02x:%02x:%02x:%02x%s", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],"\0");
             memset(s_eth_add[i], 0, COL);
                strcpy((char*)s_eth_add[i],(char*)buffer1);
                s_eth_add[i][(int)strlen((char*)buffer1)] = '\0';

                memset(d_eth_add[i], 0, COL);
                strcpy((char*)d_eth_add[i],(char*)buffer2);
                d_eth_add[i][(int)strlen((char*)buffer2)] = '\0';

        unsigned char octet[OCTETS_IN_IP] = {0,0,0,0};
        unsigned char octet1[OCTETS_IN_IP] = {0,0,0,0};
        if ( ntohs(eth->h_proto) == ETH_P_IP) {
        memset(buffer1, 0,BUFFER_SIZE);
        memset(buffer2, 0,BUFFER_SIZE);
        for (k=0; k<OCTETS_IN_IP; k++)
        {
                octet[k] = ( ip->saddr >> (k*BYTE_LEN) ) & IP_MASK;
        }

        snprintf((char*)buffer1, BUFFER_SIZE,"%d.%d.%d.%d  ",octet[0],octet[1],octet[2],octet[3]);

        for (k=0; k<OCTETS_IN_IP; k++)
        {
                octet1[k] = ( ip->daddr >> (k*BYTE_LEN) ) & IP_MASK;
        }
        snprintf((char*)buffer2, BUFFER_SIZE,"%d.%d.%d.%d  ",octet1[0],octet1[1],octet1[2],octet1[3]);

                memset(s_ip_add[i], 0, COL);
                strcpy((char*)s_ip_add[i],(char*)buffer1);
                s_ip_add[i][(int)strlen((char*)buffer1)] = '\0';

                memset(d_ip_add[i], 0, COL);
                strcpy((char*)d_ip_add[i],(char*)buffer2);
                d_ip_add[i][(int)strlen((char*)buffer2)] = '\0';
        }
     /* Check for ARP participants */
        if ( ntohs(eth->h_proto) == ETH_P_ARP) {

        snprintf((char*)buffer3,BUFFER_SIZE,"%02x:%02x:%02x:%02x:%02x:%02x / %d.%d.%d.%d%s", arp->__ar_sha[0], arp->__ar_sha[1], arp->__ar_sha[2],arp->__ar_sha[3], arp->__ar_sha[4], arp->__ar_sha[5],arp->__ar_sip[0],arp->__ar_sip[1],arp->__ar_sip[2],arp->__ar_sip[3],"\0");
                memset(arp_table[i], 0, COL);
                strcpy((char*)arp_table[i],(char*)buffer3);
                arp_table[i][(int)strlen((char*)buffer3)] = '\0';


        }

}

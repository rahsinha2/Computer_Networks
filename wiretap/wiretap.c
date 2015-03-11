#include "wiretap.h"
int count = 0;
int count2 = 0;
unsigned int smallest = ETHER_MAX_LEN;
unsigned int largest = 0;
unsigned int average = 0;
struct timeval ts1;
struct timeval ts2;
struct iphdr * ip = NULL;
struct ethhdr * eth = NULL;
struct arphdr_mod * arp = NULL;
struct tcphdr * tcp = NULL;
struct udphdr * udp = NULL;
struct icmphdr * icmp = NULL;
struct tcp_option_t * tcp_opt = NULL;
unsigned char s_eth_add[ROW][COL];
unsigned char s_ip_add[ROW][COL];
unsigned char d_eth_add[ROW][COL];
unsigned char d_ip_add[ROW][COL];
unsigned char arp_table[ROW][COL];
int net_proto[ROW];
int trans_proto[ROW];
int ttl[ROW];
char s_date[time_len];
int s_udp[ROW];
int d_udp[ROW];
int s_tcp[ROW];
int d_tcp[ROW];
int icmp_code[ROW];
int icmp_type[ROW];
uint8_t* opt = NULL;
int fin = 0, syn = 0, rst = 0, psh = 0, ack = 0, urg = 0;
int t_0 = 0, t_1 = 0, t_2 = 0, t_3 =0, t_4 = 0, t_5 =0, t_8 = 0, t_10 = 0; 

int main(int argc, char ** argv)
{
	wt_data object;
	int main_packet_count = 0;
	parse_args(argc, argv, &object);			// Parsing the command line arguments
	printf("\nFilename 		= 	%s\n", object.filename);	
	char errbuf[PCAP_ERRBUF_SIZE]; 		
    	pcap_t *handle = NULL;
	handle = pcap_open_offline(object.filename, errbuf);   	//open pcap file 
	int returnVal = 0;

	if (handle == NULL) { 
      		fprintf(stderr,"Couldn't open pcap file %s: %s\n", object.filename, errbuf); 
      		exit(1); 
    	} 

	/*** Now pcap file is successfully opened *****/

	returnVal = pcap_datalink(handle);
	
	if(returnVal != DLT_EN10MB)	{			// Check if data is captured from ethernet
		printf("\nERROR: pcap capture not from Ethernet\n");
		pcap_close(handle);
		exit(1);
	}	

	pcap_loop(handle, -1, read_packet, NULL);
	printf("=========PACKET CAPTURE SUMMARY=========\n\n");	
	printf("Capture start date   	=	%s\n",s_date);
	printf("Capture duration   	=	%ld seconds\n", ts2.tv_sec - ts1.tv_sec);
	printf("Pacekts in capture   	=	%d\n",count);
	printf("Minimum Packet Size    	=       %d\n",smallest);
	printf("Maximum Packet Size    	=       %d\n",largest);
	printf("Average Packet Size    	=	%.2f\n", (float)(((float)average)/count));
	pcap_close(handle);					// Close pcap file

	/* Now open pcap file for actaul extraction */
	handle = pcap_open_offline(object.filename, errbuf);   	//open pcap file 
	
	main_packet_count = count;
	returnVal = pcap_datalink(handle);
	
	count = 0;
	pcap_loop(handle, -1, extract_packet, NULL);	// extracting everything from pcap file

	printf("\n=========LINK LAYER=========\n");	
	printf("\n-----Source Ethernet Address-----\n\n");
	print_count(s_eth_add,main_packet_count);	//print source ether addresses

	printf("\n-----Destination Ethernet Address-----\n\n");
	print_count(d_eth_add,main_packet_count);	//printing destination ethernet addresses

	printf("\n=========NETWORK LAYER=========\n");	
	printf("\n-----Network layer protocols-----\n\n");
	print_count_int(net_proto,main_packet_count);	//printing network protocols
	
	printf("\n-----Source IP Address-----\n\n");
	print_count(s_ip_add,main_packet_count);	//printing source IP addresses

	printf("\n-----Destination IP Address-----\n\n");	//printing destination IP addresses
	print_count(d_ip_add,main_packet_count);

	printf("\n-----TTL of packets-----\n\n");	//printing TTL of packets
	print_count_ttl(ttl,main_packet_count);
	
	printf("\n-----Unique ARP Participants-----\n\n");	//printing unique arp participants
	print_count(arp_table,main_packet_count);
	
	printf("\n=========TRANSPORT LAYER=========\n");	
	printf("\n-----Transport layer protocols-----\n\n");	
	print_count_int(trans_proto,main_packet_count);		//printing transport layer protocols
	
	printf("\n=========TRANSPORT LAYER TCP=========\n");	
	printf("\n-----Source TCP ports-----\n\n");
	print_count_int(s_tcp,main_packet_count);		//printing source TCP ports
	
	printf("\n-----Destination TCP ports-----\n\n");
	print_count_int(d_tcp,main_packet_count);		//printing destination TCP ports

	printf("\n-----TCP Flags-----\n\n");
	print_tcp_flags();					//printing TCP flags

	printf("\n-----TCP Options-----\n\n");
	print_tcp_options();					//printing TCP options

	printf("\n=========TRANSPORT LAYER UDP=========\n");	
	printf("\n-----Source UDP ports-----\n\n");
	print_count_int(s_udp,main_packet_count);		//printing source UDP ports
	
	printf("\n-----Destination UDP ports-----\n\n");
	print_count_int(d_udp,main_packet_count);		//printing destinatio UDP ports

	printf("\n=========TRANSPORT LAYER ICMP=========\n");	
	printf("\n-----ICMP types-----\n\n");
	print_count_int(icmp_type,main_packet_count);		//printing ICMP types
	
	printf("\n-----ICMP codes-----\n\n");
	print_count_int(icmp_code,main_packet_count);		//printing ICMP codes
	
	pcap_close(handle);					// Close pcap file
	return 0;
}

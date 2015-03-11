#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/ip_icmp.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include "bt_lib.h"
#include "bt_setup.h"
#include <openssl/sha.h>
int  counter = 1;
int sockfd = 0;
FILE* fp_log = NULL;	//file descriptor of torrent log file

void sig_handler(int signo)
{
	if (signo == SIGINT) {
   		printf(" Received SIGINT. Exiting\n");
  		if(seeder == 1 && leecher == 1)  {	/* We are on leecher */
			send_crash_signal_to_seeder(); 
			fprintf(fp_log, "Crash encountered");
			fclose(fp_log);
		}
		exit(0);
  	}	
  	else if (signo == SIGQUIT) {
		fclose(fp_log);
        	printf("Received SIGQUIT. Exiting\n");
  	 	exit(0);
	}
}
int main (int argc, char * argv[]){

  bt_args_t bt_args;
  bt_info_t torrent_info; //for holding torrent info
  bt_request_t req_msg;   //request message
  char string_read[50];
  int i = 0;
  int integer_read = 0;
  int returnVal = 0;
  int bytesRead = 0;
  int p = 0;
  int q = 0;
  int index = 0;
  int offset = 0;
  int downloaded = 0;
  int crash_flag = 0;
  size_t torrent_file_size =0;	// for storing the size of the torrent file
  size_t bytesReceived = 0;
  FILE* fp = NULL;	//file descriptor of torrent file
  FILE* datafile   = NULL;
  char* torrent_file_string = NULL;

/* Registering for signal handlers */
	if (signal(SIGINT, sig_handler) == SIG_ERR)
  		printf("\ncan't catch SIGINT\n");
        if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        	printf("\ncan't catch SIGQUIT\n");

  parse_args(&bt_args, argc, argv);
	
	fp_log = fopen(bt_args.log_file, "w+");     /* Open file on leecher to write */

	if (fp_log == NULL) {
                printf("\nERROR: Unable to open file..Please check file name.\n");
                exit(1);
        }
			fprintf(fp_log,"%s",timestamp());
			fwrite("Start of log file\n",1,(int)strlen("Start of log file\n"),fp_log);   /* Write to file */

  unsigned char*  digest =  (unsigned char*)malloc(100);
	memset(digest, 0, 100);

  bt_bitfield_t bt;
  unsigned char message[MESSAGE_SIZE];
  unsigned char piece_buffer[PIECE_BUFFER_SIZE]; 
  unsigned char temp_buffer[PIECE_BUFFER_SIZE];

printf("%s",timestamp());
  
  if(bt_args.verbose){
    printf("Args:\n");
    printf("verbose:%d\n",bt_args.verbose);
    printf("log_file:%s\n",bt_args.log_file);
    printf("torrent_file:%s\n",bt_args.torrent_file);

    for(i=0;i<MAX_CONNECTIONS;i++){
      if(bt_args.peers[i] != NULL)
        print_peer(bt_args.peers[i]);
	if(i == 0 && bt_args.peers[0] !=NULL)
		printf("This is my own ID\n");
    }
    
  }

  //read and parse the torrent file here
	fp = fopen(bt_args.torrent_file, "r");     /* Open file on client to send */
        if (fp == NULL) {
                printf("\nERROR: Unable to open torrent file..Please check file name.\n");
                exit(1);
        }

	fseek(fp, 0, SEEK_END);			/* Finding out torrent file size */
	torrent_file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

  	torrent_file_string = malloc(torrent_file_size+1);	/* to store torrent file content in a dynamic array */
	memset(torrent_file_string, 0, torrent_file_size+1);
	bytesRead = fread(torrent_file_string,1,torrent_file_size,fp);	/* reading from torrent file */	
	
	
	char * sub = strstr(torrent_file_string,"info"); /* locating info part in the torrent file */
	char * temp = sub;
 	char * info_hash = NULL;	
	if (sub == NULL) {
		printf("\nERROR: Info part not found in torrent file\n");
		exit(1);
	}
	
	temp = temp + strlen("info");
	info_hash = temp;
	if ( temp[0] != 'd') {
		printf("\nERROR: Error in torrent file content\n");
		exit(1);
	}	
	
	temp++;
	
	sscanf(temp,"%d",&integer_read);	/* reading field length */

	while(temp[0] != ':') {
		temp++;
	}	

	temp++;
	strncpy(string_read,temp,integer_read);		/* keyword length */
	
	temp+=integer_read;	

	if ( temp[0] == 'i') {
	}	
	
	temp++;
	
	sscanf(temp,"%d",&integer_read);	/* reading length value */
	torrent_info.length  = integer_read;
	
	while(temp[0] != 'e') {
		temp++;
	}	

	temp++;
		
	sscanf(temp,"%d",&integer_read);

	while(temp[0] != ':') {
		temp++;
	}	

	temp++;
	memset(string_read,0,50);
	strncpy(string_read,temp,integer_read);		/* keyword name */
	
	temp+=integer_read;	
	
	sscanf(temp,"%d",&integer_read);

	while(temp[0] != ':') {
		temp++;
	}	
	
	temp++;
	memset(string_read,0,50);
	strncpy(string_read,temp,integer_read);		/* Filename */
	memset(torrent_info.name,0,FILE_NAME_MAX);
	strncpy(torrent_info.name,temp,integer_read);
	temp+=integer_read;
	
	sscanf(temp,"%d",&integer_read);
	
	while(temp[0] != ':') {
		temp++;
	}	
	
	temp++;
	memset(string_read,0,50);
	strncpy(string_read,temp,integer_read);	/* keyword piece length */

	temp+=integer_read;

	if ( temp[0] == 'i') {
	}	
	
	temp++;
	
	sscanf(temp,"%d",&integer_read);	/* Piece length */
	torrent_info.piece_length  = integer_read;
	
	while(temp[0] != 'e') {
		temp++;
	}	

	temp++;
		
	sscanf(temp,"%d",&integer_read);

	while(temp[0] != ':') {
		temp++;
	}	

	temp++;
	memset(string_read,0,50);
	strncpy(string_read,temp,integer_read);	/* keyword pieces */
	
	temp+=integer_read;
	
	sscanf(temp,"%d",&integer_read);	/* Number of pieces */
	torrent_info.num_pieces = integer_read/20;
	
	
	while(temp[0] != ':') {
		temp++;
	}	

	temp++;
	
	
	/* Malloc for the 2D array containg hash of all the pieces */
  		
	torrent_info.piece_hashes  =  (unsigned char**)(malloc(torrent_info.num_pieces*sizeof(unsigned char*)));

	for (i = 0; i<torrent_info.num_pieces; i++) {
		torrent_info.piece_hashes[i] = (unsigned char*)malloc(25);	
  		memset(torrent_info.piece_hashes[i],0,25);	
		strncpy((char*)torrent_info.piece_hashes[i],temp,20);		/* Copy digest */
		torrent_info.piece_hashes[i][20]='\0';
		temp+=20;
	}
				
	printf("Torrent file parsed\n");
	fprintf(fp_log,"%s",timestamp());
	fprintf(fp_log,"Torrent file parsed\n");
	fclose(fp);

	bt_args.bt_info = &torrent_info;
	
	fprintf(fp_log,"File Name 		= %s\n",torrent_info.name);
	fprintf(fp_log,"File Length 		= %d Bytes\n",torrent_info.length);
	fprintf(fp_log,"Piece Length 		= %d Bytes\n",torrent_info.piece_length);
	fprintf(fp_log,"Number of Pieces 	= %d\n",torrent_info.num_pieces);

  if(bt_args.verbose){
    // print out the torrent file arguments here

printf("%s",timestamp());

	printf("\nFile Name 		= %s\n",torrent_info.name);
	printf("File Length 		= %d Bytes\n",torrent_info.length);
	printf("Piece Length 		= %d Bytes\n",torrent_info.piece_length);
	printf("Number of Pieces 	= %d\n",torrent_info.num_pieces);
	printf("Digest of each piece is = ");
	int j=0;				/* Printing hash of all the pieces */
	for (i = 0; i<torrent_info.num_pieces; i++) {
		printf("\nPiece [%d] hash = ", i+1);
	for (j = 0; j < 20; j++) 
          	printf("%02x ", torrent_info.piece_hashes[i][j]);
		
	}
  }
	
	if (save_file_flag !=1  && leecher == 1)
		strncpy(bt_args.save_file,torrent_info.name,FILE_NAME_MAX);
	
printf("\n%s",timestamp());
	if (seeder == 1 && leecher == 1) {
		printf("\nWe are on leecher\n");
			fprintf(fp_log,"%s",timestamp());
			fwrite("This is leecher\n",1,(int)strlen("This is leecher\n"),fp_log);   /* Write to file */
	}
	else if (seeder == 1 && leecher == 0) {
		printf("\nWe are on seeder\n");
			fprintf(fp_log,"%s",timestamp());
			fwrite("This is seeder\n",1,(int)strlen("This is seeder\n"),fp_log);   /* Write to file */
	}
	
	unsigned char handshake[HANDSHAKE_SIZE+1];
	unsigned char * temp_hs = NULL;		/* using old variable from pasrsing temp */
	temp_hs = handshake;
	memset(handshake, 0, HANDSHAKE_SIZE);
	char prefix = 19;
	handshake[0] = prefix;
	temp_hs++;
	strcpy((char*)temp_hs,"BitTorrent Protocol");	
	temp_hs+=19;
	for(i=20; i<=27; i++)
	handshake[i] = '0';
	temp_hs+=8;

	SHA1((unsigned char *) info_hash, strlen(info_hash), (unsigned char *) digest);	
	digest[20]='\0';

	strcpy((char*)temp_hs,(char*)digest);// Value to be hashed is  info field
	temp_hs+=20;
	strncpy((char*)temp_hs,(char*)bt_args.peers[0]->id,20);	
	handshake[HANDSHAKE_SIZE] = '\0';
	
	/*********Seeder**********/
	if (seeder == 1 && leecher == 0 ) {
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        	if ( sockfd < 0) {
                	printf("\nERROR: Socket() creation failed\n");
                	exit(1);
        	}   

        /* Bind to local address */
        	if (bind(sockfd, (struct sockaddr*) &(bt_args.peers[0]->sockaddr), sizeof(bt_args.peers[0]->sockaddr)) < 0 ) { 
                	printf("\nERROR: Bind() operation failed\n");
                	exit(1);
        	}   

        /* Make socket listen to incoming connections */
    
   	     if (listen(sockfd, MAX_CONNECTIONS) < 0) {
        	        printf("\nERROR: Listen() operation failed\n");
                	exit(1);
        	}   
	}
	
	else if (seeder == 1 && leecher == 1) {

int yes = 1;
	/* Reliable socket creation */

        	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        	if ( sockfd < 0) {
                	printf("\nERROR : Socket() creation failed\n");
                	exit(1);
        	}
/***********************************************************/

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {	/* Tell the kernel that you are willing to re-use the port*/
                	printf("\nERROR: Setsockopt() operation failed\n");			/* But check this for 1 seeder N leecher. Allows all to bind */
    			exit(1);
		}
        /* Bind to local address on leecher */
        	if (bind(sockfd, (struct sockaddr*) &(bt_args.peers[0]->sockaddr), sizeof(bt_args.peers[0]->sockaddr)) < 0 ) { 
                	printf("\nERROR: Bind() operation failed\n");
                	exit(1);
        	}   
        /* Connect to  seeder */

        	if (connect(sockfd, (struct sockaddr*) &(bt_args.peers[1]->sockaddr), sizeof(bt_args.peers[1]->sockaddr)) < 0 ) { 
                	printf("\nERROR : Connect() failed\n");
                	exit(1);
        }
	
	size_t bytesSent = 0;
/*******************we need to send handshake message here*****************/
	
	bytesSent = write(sockfd, handshake, (int)strlen((char*)handshake));
	printf("\n\nHandshake INIT message sent to Seeder");
	fprintf(fp_log,"%s",timestamp());
	fprintf(fp_log,"HANDSHAKE INIT message sent to Seeder\n");
		fflush(stdout);
/******************checking for seeders reply to handshake init ************/	
	unsigned char receivedBuffer[HANDSHAKE_SIZE+1];	
	unsigned char* temp = NULL;
	memset(receivedBuffer, 0, HANDSHAKE_SIZE);
	bytesReceived = read(sockfd, receivedBuffer, HANDSHAKE_SIZE);
		fflush(stdout);
	receivedBuffer[strlen((char*)receivedBuffer)] = '\0';
	fflush(stdout);
	if( strcmp((char*)receivedBuffer,"terminate")==0) {
		printf("\nHanshake not successfull. Exiting\n");
		exit(1);								/***change this ****/
	}
	printf("\n\nLeecher received handshake INIT response from Seeder\n");
			fprintf(fp_log,"%s",timestamp());
			fwrite("HANDSHAKE INIT response received\n",1,(int)strlen("HANDSHAKE INIT response received\n"),fp_log);   /* Write to file */
	fflush(stdout);

	/* Check for handshake consistency on leecher */
	int handshake_success = 0;
	unsigned char check_buffer[68];
	memset(check_buffer, 0, 68);
	temp = receivedBuffer;
	char prefix = 19;
	if (temp[0] == prefix) {
			fprintf(fp_log,"%s",timestamp());
			fwrite("1st byte = 19 TRUE\n",1,(int)strlen("1st byte = 19 TRUE\n"),fp_log);   /* Write to file */
  	if(bt_args.verbose)
		printf("\n19 is the 1st byte\n");	
		handshake_success++;
	}
	temp++;
	fflush(stdout);
	strncpy((char*)check_buffer, (char*)temp,19);
  	if(bt_args.verbose)
	printf("Protocol = %s", (char*)check_buffer);	
	fflush(stdout);
	if (strcmp((char*)check_buffer,"BitTorrent Protocol") == 0) {
			fwrite("Protocol = BitTorrent Protocol TRUE\n",1,(int)strlen("Protocol = BitTorrent Protocol TRUE\n"),fp_log);   /* Write to file */
  	if(bt_args.verbose)
		printf("-->Protocol Match Successfull\n");
		handshake_success++;
	}
	fflush(stdout);
	memset(check_buffer, 0, 68);
	temp+=19+8;
	strncpy((char*)check_buffer, (char*)temp,20);
  	if(bt_args.verbose)
	printf("\nSHA1 hash of bencoded info from seeder\n");	
	fflush(stdout);
	int check_flag = 0;
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++) 
          printf("%02x ", check_buffer[i]);
  	if(bt_args.verbose)
	printf("\n");
	fflush(stdout);

  	if(bt_args.verbose)
	printf("\nSHA1 hash of bencoded info from torrent file\n");	
	fflush(stdout);
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++)
                        printf("%02x ", digest[i]);
  	if(bt_args.verbose)
	printf("\n");
	fflush(stdout);
	for (i = 0; i < 20; i++) {
        	if(digest[i] != check_buffer[i]) {
			check_flag =1;
			break;
		}
	} 
	fflush(stdout);
	
	if(check_flag == 1) {
			fwrite("SHA1 digest mismatch\n",1,(int)strlen("SHA1 digest mismatch\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nSHA1 digest mismatch\n");
	}
	else if (check_flag ==0 ) {
			fwrite("SHA1 digest match Success TRUE\n",1,(int)strlen("SHA1 digest match Success TRUE\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nSHA1 digest match successfull\n");
		handshake_success++;
	}

	temp+=20;
	memset(check_buffer, 0, 68);
	strncpy((char*)check_buffer, (char*)temp,20);
  		if(bt_args.verbose)
	printf("\nID from seeder\n");
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++) 
          printf("%02x", check_buffer[i]);
  	if(bt_args.verbose)
	printf("\n");
	fflush(stdout);

	/******check for valid seeder ID*********/	
	unsigned char seeder_id[25];
	memset(seeder_id,0,25);
	calc_id(inet_ntoa(bt_args.peers[1]->sockaddr.sin_addr), ntohs(bt_args.peers[1]->sockaddr.sin_port), (char*)seeder_id);
	seeder_id[20]='\0';
  	if(bt_args.verbose)
	printf("\nID of seeder from self calculation\n");
	fflush(stdout);
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++) 
          printf("%02x", seeder_id[i]);
  	if(bt_args.verbose)
	printf("\n");
	
	fflush(stdout);
	check_flag = 0;
	for (i = 0; i < 20; i++) {
        	if(check_buffer[i] != seeder_id[i]) {
			check_flag =1;
			break;
		}
	} 
	fflush(stdout);
	
	if(check_flag == 1) {
			fwrite("Seeder ID mismatch\n",1,(int)strlen("Seeder ID mismatch\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nSeeder ID  mismatch\n");
	}
	else if (check_flag ==0 ) {
			fwrite("Seeder ID match Success TRUE\n",1,(int)strlen("Seeder ID match Success TRUE\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nSeeder ID match successfull\n");
		handshake_success++;
	}
	fflush(stdout);
	
	if(handshake_success == 4) {  	/* means all 4 hanshake parameters are good */
		printf("\nHandshake Successfull");
			fprintf(fp_log,"%s",timestamp());
			fwrite("HANDSHAKE SUCCESS\n",1,(int)strlen("HANDSHAKE SUCCESS\n"),fp_log);   /* Write to file */
	}
	else	{
		printf("\nHandshake UnSuccessfull");
			fprintf(fp_log,"%s",timestamp());
			fwrite("HANDSHAKE UNSUCCESSFULL\n",1,(int)strlen("HANDSHAKE UNSUCCESSFULL\n"),fp_log);   /* Write to file */
	}	
	fflush(stdout);
	printf("\nSeeder  IP   : %s\n",
           inet_ntoa(bt_args.peers[1]->sockaddr.sin_addr));
	fflush(stdout);
	printf("Seeder  port : %u\n", ntohs(bt_args.peers[1]->sockaddr.sin_port));
			fprintf(fp_log,"%s",timestamp());
			fprintf(fp_log,"Seeder  IP   : %s ",
           			inet_ntoa(bt_args.peers[1]->sockaddr.sin_addr));
			fprintf(fp_log,"Seeder  port : %u\n", ntohs(bt_args.peers[1]->sockaddr.sin_port));
			fprintf(fp_log,"Seeder  ID   : ");
			for (i = 0; i < 20; i++) 
          			fprintf(fp_log,"%02x", check_buffer[i]);
			fprintf(fp_log,"\n");
/*****Handhsake over on leecher ************/


	memset(message,0,MESSAGE_SIZE);
	bytesReceived = read(sockfd, message, MESSAGE_SIZE);
	receive_bitfield_message(&bt, message);		// prints the bit field on received from seeder and self populate bitfield structure
			fwrite(timestamp(),1,(int)strlen(timestamp()),fp_log);   /* Write to file */
			fwrite("Received BITFIELD MESSAGE from seeder : Type[5] : ",1,(int)strlen((char*)("Received BITFIELD MESSAGE from seeder : Type[5] : ")),fp_log);   /* Write to file */
			fwrite(bt.bitfield,1,(int)strlen((char*)bt.bitfield),fp_log);   /* Write to file */
			fwrite("\n",1,strlen("\n"),fp_log);	

	create_message(message,BT_INTERESTED);		//creating and sending interested message to seeder	

	
	bytesSent = write(sockfd, message, (int)strlen((char*)message));	//sent interested message
			fwrite("Sent INTERESTED MESSAGE to seeder : Type[2]\n",1,(int)strlen("Sent INTERESTED MESSAGE to seeder : Type[2]\n"),fp_log);   /* Write to file */
	
	/* here we receive unchoke message */
	memset(message,0,MESSAGE_SIZE);
	bytesReceived = read(sockfd, message, MESSAGE_SIZE);
	fwrite("Received UNCHOKE MESSAGE from seeder : Type[1]\n",1,(int)strlen("Received UNCHOKE MESSAGE from seeder : Type[1]\n"),fp_log);   /* Write to file */
	returnVal = receive_message(message);
	if (returnVal == BT_UNCHOKE) {				//set peer to interested and Unchoked	
		bt_args.peers[1]->choked = 0;			//set seeder to interested and unchoked	
		bt_args.peers[1]->interested = 1;		
	}
	/****** Here we are done with setting choke and interested values after receiving unchoke message from seeder *****/
	/*****Now go to whille loop of client for continuously sending request messages*****/

	}


  //main client loop
  while(1){
counter = 0;
crash_flag = 0;
downloaded = 0;
    //try to accept incoming connection from new peer
       
    
    //poll current peers for incoming traffic
    //   write pieces to files
    //   udpdate peers choke or unchoke status
    //   responses to have/havenots/interested etc.
    
    //for peers that are not choked
    //   request pieaces from outcoming traffic

    //check livelenss of peers and replace dead (or useless) peers
    //with new potentially useful peers
    
    //update peers, 
	if (seeder == 1 && leecher == 0) {
	struct sockaddr_in clientIP;                    /* Client IP Adress */
	socklen_t       clientLen = sizeof(clientIP);   /* Length of client address*/

        /* Accept client connection */
printf("%s",timestamp());
	
	printf("Waiting for client to connect.....\n");
        int clientSock = accept(sockfd, (struct sockaddr *) &clientIP, &clientLen);
        	if (clientSock < 0) {
                	printf("\nERROR: Accept() operation failed\n");
                	exit(1);
        	}

	/* populate leecher detail on seeder in the peer_t structure*/
	bt_args.peers[1] = malloc(sizeof(peer_t));
	

	unsigned char receivedBuffer[HANDSHAKE_SIZE+1];	
	unsigned char* temp = NULL;
	memset(receivedBuffer, 0, HANDSHAKE_SIZE);
	bytesReceived = read(clientSock, receivedBuffer, HANDSHAKE_SIZE);
	receivedBuffer[68] = '\0';
	printf("\nSeeder received handshake INIT from leecher\n");
	fprintf(fp_log,"%s",timestamp());
	fprintf(fp_log,"Seeder received HANDSHAKE INIT from leecher\n");
	
	/* Check for handshake consistency on seeder */
	int handshake_success = 0;
	unsigned char check_buffer[68];
	memset(check_buffer, 0, 68);
	temp = receivedBuffer;
	char prefix = 19;
	if (temp[0] == prefix) {
			fprintf(fp_log,"%s",timestamp());
			fwrite("1st byte = 19 TRUE\n",1,(int)strlen("1st byte = 19 TRUE\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("19 is the 1st byte\n");	
		handshake_success++;
	}
	temp++;
	fflush(stdout);
	strncpy((char*)check_buffer, (char*)temp,19);
  	if(bt_args.verbose)
	printf("Protocol = %s", (char*)check_buffer);	
	fflush(stdout);
	if (strcmp((char*)check_buffer,"BitTorrent Protocol") == 0) {
			fwrite("Protocol = BitTorrent Protocol TRUE\n",1,(int)strlen("Protocol = BitTorrent Protocol TRUE\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("-->Protocol Match Successfull\n");
		handshake_success++;
	}
	fflush(stdout);
	memset(check_buffer, 0, 68);
	temp+=19+8;
	strncpy((char*)check_buffer, (char*)temp,20);
  	if(bt_args.verbose)
	printf("\nSHA1 hash of bencoded info from leecher\n");	
	fflush(stdout);
	int check_flag = 0;
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++) 
          printf("%02x ", check_buffer[i]);
  	if(bt_args.verbose)
	printf("\n");
	fflush(stdout);

  	if(bt_args.verbose)
	printf("\nSHA1 hash of bencoded info from torrent file\n");	
	fflush(stdout);
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++)
                        printf("%02x ", digest[i]);
  	if(bt_args.verbose)
	printf("\n");
	fflush(stdout);
	for (i = 0; i < 20; i++) {
        	if(digest[i] != check_buffer[i]) {
			check_flag =1;
			break;
		}
	} 
	fflush(stdout);
	
	if(check_flag == 1) {
		fwrite("SHA1 digest mismatch\n",1,(int)strlen("SHA1 digest mismatch\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nSHA1 digest mismatch\n");
	}
	else if (check_flag ==0 ) {
		fwrite("SHA1 digest match Success TRUE\n",1,(int)strlen("SHA1 digest match Success TRUE\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nSHA1 digest match successfull\n");
		handshake_success++;
	}

	temp+=20;
	memset(check_buffer, 0, 68);
	strncpy((char*)check_buffer, (char*)temp,20);
  		if(bt_args.verbose)
	printf("\nID from leecher\n");
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++) 
          printf("%02x", check_buffer[i]);
  	if(bt_args.verbose)
	printf("\n");
	fflush(stdout);

	/******check for valid leecher ID*********/	
	unsigned char leecher_id[25];
	memset(leecher_id,0,25);
	calc_id(inet_ntoa(clientIP.sin_addr), ntohs(clientIP.sin_port), (char*)leecher_id);
	leecher_id[20]='\0';
  		if(bt_args.verbose)
	printf("\nID of leecher from self calculation\n");
	fflush(stdout);
  	if(bt_args.verbose)
	for (i = 0; i < 20; i++) 
          printf("%02x", leecher_id[i]);
  	if(bt_args.verbose)
	printf("\n");
	

	check_flag = 0;
	for (i = 0; i < 20; i++) {
        	if(check_buffer[i] != leecher_id[i]) {
			check_flag =1;
			break;
		}
	} 
	fflush(stdout);
	
	init_peer(bt_args.peers[1], (char*)leecher_id, inet_ntoa(clientIP.sin_addr), ntohs(clientIP.sin_port));  // storing the client details on my peer structure using clientSock	

	if(check_flag == 1) {
		fwrite("Leecher ID mismatch\n",1,(int)strlen("Leecher ID mismatch\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nLeecher ID  mismatch\n");
	}
	else if (check_flag ==0 ) {
		fwrite("Leecher ID match Success TRUE\n",1,(int)strlen("Leecher ID match Success TRUE\n"),fp_log);   /* Write to file */
  		if(bt_args.verbose)
		printf("\nLeecher ID match successfull\n");
		handshake_success++;
	}
	
	fflush(stdout);
	printf("\nSeeder received handshake INIT from \n");
	printf("peer client IP   : %s\n",
           inet_ntoa(clientIP.sin_addr));
	fflush(stdout);
	printf("peer client port : %u\n", ntohs(clientIP.sin_port));
	fflush(stdout);
	printf("peer client id   : ");
	for (i = 0; i < 20; i++) 
          printf("%02x", check_buffer[i]);
	printf("\n");
	fflush(stdout);
	fflush(stdout);

	/******NOW send handshake response*********/
	
	memset(receivedBuffer, 0, HANDSHAKE_SIZE);
	if(handshake_success == 4) { 	/* mean all 4 handhshake parameters received are good */
		strcpy((char*)receivedBuffer, "INIT passed\0");
		printf("\nHandshake Successfull");
		fprintf(fp_log,"%s",timestamp());
		fwrite("HANDSHAKE SUCCESS\n",1,(int)strlen("HANDSHAKE SUCCESS\n"),fp_log);   /* Write to file */
	}
	else	{
		strcpy((char*)receivedBuffer, "INIT failed\0");
		printf("\nHandshake UnSuccessfull");
		fprintf(fp_log,"%s",timestamp());
		fwrite("HANDSHAKE UNSUCCESSFULL\n",1,(int)strlen("HANDSHAKE UNSUCCESSFULL\n"),fp_log);   /* Write to file */
	}
	size_t bytesSent = 0;	
	if(handshake_success == 4) { 	/* mean all 4 handhshake parameters received are good */
		bytesSent = write(clientSock, handshake, (int)strlen((char*)handshake));
		printf("\nHandhshake INIT response sent to leecher\n");
		fflush(stdout);
		}
		else {
		bytesSent = write(clientSock, "terminate", (int)strlen("terminate"));
		}
			
		fprintf(fp_log,"%s",timestamp());
		fprintf(fp_log,"Leecher  IP   : %s ",
           		inet_ntoa(clientIP.sin_addr));
		fprintf(fp_log,"Leecher  port : %u\n", ntohs(clientIP.sin_port));
		fprintf(fp_log,"Leecher  ID   : ");
		for (i = 0; i < 20; i++) 
          		fprintf(fp_log,"%02x", check_buffer[i]);
		fprintf(fp_log,"\n");
	
	/******************Handshake over on seeder ****************/

	/*****malloc for bitfield*****/

	get_bitfield(&bt_args,  &bt);
	create_bitfield_message(&bt, message);

	bytesSent = write(clientSock, message, (int)strlen((char*)message));	// bitfield message sent
	fprintf(fp_log,"%s",timestamp());
	fprintf(fp_log,"BITFIELD MESSAGE sent to Leecher : Type[5] : %s\n",bt.bitfield);	
	memset(message,0,MESSAGE_SIZE);

	bytesReceived = read(clientSock, message, MESSAGE_SIZE);	// reading interested message from leecher
	fwrite("Received INTERESTED MESSAGE from Leecher : Type[2]\n",1,(int)strlen("Received INTERESTED MESSAGE from Leecher : Type[2]\n"),fp_log);   /* Write to file */
	receivedBuffer[68] = '\0';
        returnVal = receive_message(message);
	
	if (returnVal == BT_INTERESTED) {				//set peer to interested and Unchoked	
		bt_args.peers[1]->choked = 0;		
		bt_args.peers[1]->interested = 1;		
	}
	create_message(message,BT_UNCHOKE);				//creating unchoke message
	bytesSent = write(clientSock, message, (int)strlen((char*)message));	// unchoke message sent
	fwrite("Sent UNCHOKE MESSAGE to Leecher : Type[1]\n",1,(int)strlen("Sent UNCHOKE MESSAGE to Leecher : Type[1]\n"),fp_log);   /* Write to file */

	/* open the file here to be sent*/
	
	datafile =fopen(bt_args.bt_info->name, "r");     /* Open file on client to send */
        if (datafile == NULL) {
                printf("\nERROR: Unable to open file..Please check file name.\n");
                exit(1);
        }

	int fileread = 0;
	/****while loop for read request messages and sending piece message****/
	for (p=0; p <(bt_args.bt_info->num_pieces); p++) {
	
	index = p;	
	
	for (q=0; q<(bt_args.bt_info->piece_length); q+= BLOCK_SIZE)	{
		
	offset = q;
	memset(message,0,MESSAGE_SIZE);
	bytesReceived = read(clientSock, message, MESSAGE_SIZE);	// reading request messages from client
	
	if (strcmp((char*)message,"crash") == 0){
		printf("\nClient has crashed. Exception handled.\n");
		fprintf(fp_log,"Client crashed");
		crash_flag = 1;
		break;
	}
		
	/* populate request message structure on seeder */
	receive_request_message(&req_msg, message);
	fprintf(fp_log,"%s",timestamp());
	fprintf(fp_log,"REQUEST MESSAGE Received : Type[6] : Index=%d Offset=%d BlockSize=%d\n", req_msg.index, req_msg.begin, BLOCK_SIZE);	
	/* populate the temp_buffer with the data from the actual file */
	
	memset(temp_buffer,0,PIECE_BUFFER_SIZE);
	fseek(datafile,0,SEEK_SET);
	fseek(datafile,(req_msg.index*(bt_args.bt_info->piece_length)+req_msg.begin), SEEK_SET);
	fileread = fread(temp_buffer,1, BLOCK_SIZE,datafile);       /* Reading from file */                   /*****please check for last character later*/
	downloaded+=fileread;	
	/* Now my bt_request_t is populated. I will create bt_piece message with this data */	
	create_piece_message(&req_msg, piece_buffer,temp_buffer, fileread);
	bytesSent = write(clientSock, piece_buffer, (int)strlen((char*)piece_buffer));
	fprintf(fp_log,"PIECE MESSAGE sent 	 : Type[7] : Index=%d Offset=%d BlockSize=%d\n", req_msg.index, req_msg.begin, fileread);	
	if (fileread == 0)		/* end of the data in a block */
		break;
	counter++;
	//printf("\n%d]Piece message sent : %s", counter++,piece_buffer);	

	}/**end inner for**/
		
	if (crash_flag == 1)
		break;		
	/***receive have message here ***/

  	fprintf(fp_log,"Sent PIECE MESSAGE      %d to Leecher | Progress = %.02f %s | Uploaded = %d Bytes\n", req_msg.index+1, (((float)(p+1))/((float)bt_args.bt_info->num_pieces)*100), "%", downloaded);
        fprintf(fp_log,"Received HAVE MESSAGE    : Type[%d]\n", BT_HAVE);
	
	}/**end outer for**/
	
	if (crash_flag ==0)
	printf("\nFile sent to leecher\n");
	fprintf(fp_log,"\nFile Upload to Leecher Complete\n");
	fclose(datafile);  /* closing file on seeder */
	} /* End of seeder block */

	if (seeder == 1 && leecher == 1) {
	/** send request message **/
	size_t bytesSent = 0;
	unsigned char*  r_digest =  (unsigned char*)malloc(100);
  	unsigned char* temp_buffer_for_sha = (unsigned char*)malloc(bt_args.bt_info->piece_length);
	memset(temp_buffer_for_sha, 0, bt_args.bt_info->piece_length);
	memset(r_digest, 0, 100);

	printf("\nStarting file transfer\n");
	datafile = fopen(bt_args.save_file, "w+");     /* Open file on leecher to write */

	if (datafile == NULL) {
                printf("\nERROR: Unable to open file..Please check file name.\n");
                exit(1);
        }
	

	/****HEre we need to fill all the piece indexes in  an array and shuffle the array******/
	int * piece_index = (int*)malloc((bt_args.bt_info->num_pieces)*sizeof(int));
	int r_index = 0;
	int r_offset = 0;	
	int sha_check = 0;
	
	shuffle_piece_index(piece_index, bt_args.bt_info->num_pieces);

	for (p=0; p <(bt_args.bt_info->num_pieces); p++) {
	
	index = piece_index[p];	

	for (q=0; q<(bt_args.bt_info->piece_length); q+= BLOCK_SIZE)	{
		
		offset = q;
		create_request_message(&req_msg,message, index, offset);		//creating request message
		bytesSent = write(sockfd, message, (int)strlen((char*)message));	//sending request  message
		fwrite(timestamp(),1,(int)strlen(timestamp()),fp_log);   /* Write to file */
		fprintf(fp_log,"REQUEST MESSAGE Sent   : Type[6] : Index=%d Offset=%d BlockSize=%d\n", index, offset, BLOCK_SIZE);	
		memset(piece_buffer,0,PIECE_BUFFER_SIZE);
		bytesReceived = read(sockfd, piece_buffer, PIECE_BUFFER_SIZE);
		
		if ( strcmp((char*)piece_buffer, "stop") == 0)
			break;
		if(bytesReceived>HEADER_SIZE) {
			extract_block_from_message(piece_buffer,temp_buffer, &r_index, &r_offset);
			if ((int)strlen((char*)temp_buffer) < BLOCK_SIZE)
				temp_buffer[((int)strlen((char*)temp_buffer)) - 1] = '\0';	// setting last byte of half filled block to \0	
			
			fseek(datafile,0,SEEK_SET);
			fseek(datafile,(r_index*(bt_args.bt_info->piece_length)+r_offset), SEEK_SET);
			fwrite(temp_buffer,1,(int)strlen((char*)temp_buffer),datafile);   /* Write to file */
			fprintf(fp_log,"Received PIECE MESSAGE : Type[7] : Index=%d Offset=%d BlockSize=%d\n", index, offset, (int)strlen((char*)temp_buffer));	
		}
		fflush(stdout);	

	}/**end inner for**/
	
	/***nth piece received. now check for SHA1 hash consistency *****/
			fseek(datafile,0,SEEK_SET);
			memset(temp_buffer_for_sha, 0, bt_args.bt_info->piece_length);
			fseek(datafile,(r_index*(bt_args.bt_info->piece_length)+0), SEEK_SET);
			int fileread = fread(temp_buffer_for_sha,1, bt_args.bt_info->piece_length,datafile);       
			downloaded += fileread;
			temp_buffer_for_sha[fileread] = '\0';
			
				
	memset(r_digest, 0, 100);
	SHA1(temp_buffer_for_sha, strlen((char*)temp_buffer_for_sha), r_digest);	
	sha_check = 0;
	
	for (i = 0; i < 20; i++) {
        	if(r_digest[i] != torrent_info.piece_hashes[r_index][i]) {
			sha_check = 1;
			break;
		}
	
	} 
  	fprintf(fp_log,"Received piece      %d from seeder | Progress = %.02f %s | Downloaded = %d Bytes\n", r_index+1, (((float)(p+1))/((float)bt_args.bt_info->num_pieces)*100), "%", downloaded);
  	printf("\nReceived piece      %d from seeder | Progress = %.02f %s | Downloaded = %d Bytes", r_index+1, (((float)(p+1))/((float)bt_args.bt_info->num_pieces)*100), "%", downloaded);
	fprintf(fp_log,"SHA check for piece %d = %d (0 = Success, 1 = Failure)\n", r_index+1, sha_check);
	if(bt_args.verbose) {
	printf("\nSHA check for piece %d = %d (0 = Success, 1 = Failure)", r_index+1, sha_check);
	printf("\n-------------------------------------------------------------------------------------");	
	}
	create_have_message(message, BT_HAVE,r_index);	
        fprintf(fp_log,"Sent HAVE MESSAGE : Type[%d]\n", BT_HAVE);

	}/**end outer for**/


	fclose(datafile);
	free(piece_index);
	free(r_digest);
	printf("\nFile downloaded\n");	
	fprintf(fp_log,"\nFile downloaded\n");	
	printf("%s",timestamp());
	fclose(fp_log);
	exit(0);
	}	
	

	fflush(stdout);

  }

  return 0;
}
void send_crash_signal_to_seeder()
{
	write(sockfd, "crash", (int)strlen("crash"));	//sending crash  message
	
}

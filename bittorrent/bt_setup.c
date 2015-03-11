#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "bt_setup.h"
#include "bt_lib.h"


/**
 * usage(FILE * file) -> void
 *
 * print the usage of this program to the file stream file
 *
 **/

void usage(FILE * file){
  if(file == NULL){
    file = stdout;
  }

  fprintf(file,
          "bt-client [OPTIONS] file.torrent\n"
          "  -h            \t Print this help screen\n"
          "  -b ip:port    \t Bind to this ip for incoming connections, ports\n"
          "                \t are selected automatically\n"
          "  -s save_file  \t Save the torrent in directory save_dir (dflt: .)\n"
          "  -l log_file   \t Save logs to log_file (dflt: bt-client.log)\n"
          "  -p ip:port    \t Instead of contacing the tracker for a peer list,\n"
          "                \t use this peer instead, ip:port (ip or hostname)\n"
          "                \t (include multiple -p for more than 1 peer)\n"
          "  -I id         \t Set the node identifier to id (dflt: random)\n"
          "  -v            \t verbose, print additional verbose info\n");
}

/**
 * __parse_peer(peer_t * peer, char peer_st) -> void
 *
 * parse a peer string, peer_st and store the parsed result in peer
 *
 * ERRORS: Will exit on various errors
 **/

void __parse_peer(peer_t * peer, char * peer_st){
  char * parse_str;
  char * word;
  unsigned short port;
  char * ip;
  char id[20];
  char sep[] = ":";
  int i;
  //need to copy becaus strtok mangels things
  parse_str = malloc(strlen(peer_st)+1);
  strncpy(parse_str, peer_st, strlen(peer_st)+1);

  //only can have 2 tokens max, but may have less
  for(word = strtok(parse_str, sep), i=0; 
      (word && i < 3); 
      word = strtok(NULL,sep), i++){

    //printf("%d:%s\n",i,word);
    switch(i){
    case 0://id
      ip = word;
      break;
    case 1://ip
      port = atoi(word);
    default:
      break;
    }

  }

  if(i < 2){
    fprintf(stderr,"ERROR: Parsing Peer: Not enough values in '%s'\n",peer_st);
    usage(stderr);
    exit(1);
  }

  if(word){
    fprintf(stderr, "ERROR: Parsing Peer: Too many values in '%s'\n",peer_st);
    usage(stderr);
    exit(1);
  }

	if(strcmp(ip,"localhost")==0)
		calc_id("127.0.0.1",port,id);
	else
		calc_id(ip,port,id);

  //build the object we need
	if(strcmp(ip,"localhost")==0)
  		init_peer(peer, id, "127.0.0.1", port);
  	else	
		init_peer(peer, id, ip, port);
  

 /*if (leecher == 1 && seeder == 0)
  	init_peer(peer, id, ip, port);
  else if (leecher ==0 && seeder == 1)
	init_seeder_id(NULL,id,ip,port);*/
  //free extra memory
  free(parse_str);

  return;
}

/**
 * pars_args(bt_args_t * bt_args, int argc, char * argv[]) -> void
 *
 * parse the command line arguments to bt_client using getopt and
 * store the result in bt_args.
 *
 * ERRORS: Will exit on various errors
 *
 **/
void parse_args(bt_args_t * bt_args, int argc,  char * argv[]){
  int ch; //ch for each flag
  int n_peers = 0;
  int i;
  seeder = 0;
  leecher = 0;

  /* set the default args */
  bt_args->verbose=0; //no verbosity
  
  //null save_file, log_file and torrent_file
  memset(bt_args->save_file,0x00,FILE_NAME_MAX);
  memset(bt_args->torrent_file,0x00,FILE_NAME_MAX);
  memset(bt_args->log_file,0x00,FILE_NAME_MAX);
  
  //null out file pointers
  bt_args->f_save = NULL;

  //null bt_info pointer, should be set once torrent file is read
  bt_args->bt_info = NULL;

  //default log file
  strncpy(bt_args->log_file,"bt-client.log",FILE_NAME_MAX);
  
  for(i=0;i<MAX_CONNECTIONS;i++){
    bt_args->peers[i] = NULL; //initially NULL
  }

  bt_args->id = 0;
  
  while ((ch = getopt(argc, argv, "hb:p:s:l:vI:")) != -1) {
    switch (ch) {
    case 'h': //help
      usage(stdout);
      exit(0);
      break;
    case 'v': //verbose
      bt_args->verbose = 1;
      break;
    case 's': //save file
      save_file_flag = 1;
      strncpy(bt_args->save_file,optarg,FILE_NAME_MAX);
      break;
    case 'l': //log file
      strncpy(bt_args->log_file,optarg,FILE_NAME_MAX);
      break;
    case 'b': //local address and port to bind to. Both on seeder and client
      seeder = 1;
      n_peers=0;
      //check if we are going to overflow
      if(n_peers > 1){ 		// Only one seeder
        fprintf(stderr,"ERROR: Can only support %d initial peers",MAX_CONNECTIONS);
        usage(stderr);
        exit(1);
      }

      bt_args->peers[n_peers] = malloc(sizeof(peer_t));
      //parse seeders own ip and port and calculate the id
      __parse_peer(bt_args->peers[n_peers], optarg);
	break;
    case 'p': //peer
      leecher = 1;
      n_peers++;
      //check if we are going to overflow
      if(n_peers > MAX_CONNECTIONS){
        fprintf(stderr,"ERROR: Can only support %d initial peers",MAX_CONNECTIONS);
        usage(stderr);
        exit(1);
      }

      bt_args->peers[n_peers] = malloc(sizeof(peer_t));

      //parse peers
      __parse_peer(bt_args->peers[n_peers], optarg);
      break;
    case 'I':
      bt_args->id = atoi(optarg);
	break;
    default:
      fprintf(stderr,"ERROR: Unknown option '-%c'\n",ch);
      usage(stdout);
      exit(1);
    }
  }

  if(argc == 1){
    	printf("ERROR: Missing command line arguments\n");
    	usage(stderr);
    	exit(1);
  }

	
	if(seeder == 1 && leecher ==0){
		if (argc> 9) {
    			printf("ERROR: Excess command line arguments\n");
    			usage(stderr);
    			exit(1);
		}
		else if (argc < 4) {
    			printf("ERROR: Missing command line arguments\n");
    			usage(stderr);
    			exit(1);
		}
	}

	if(seeder == 1 && leecher ==1) {
		if (argc > 12) {
    			printf("ERROR: Excess command line arguments\n");
    			usage(stderr);
    			exit(1);
		}
		else if (argc < 6) {
    			printf("ERROR: Missing command line arguments\n");
    			usage(stderr);
    			exit(1);
		}

	}	
  //copy torrent file over
  strncpy(bt_args->torrent_file,argv[optind],FILE_NAME_MAX);    // argv[0] when argc -= optind and argv += optind are not commented 

  return ;
}
char * timestamp()
{
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    //printf("%s",asctime( localtime(&ltime) ) );
    return(asctime( localtime(&ltime) ) );
}
void shuffle_piece_index(int * piece_index, int num_pieces)
{
        int i = 0;
        int r = 0;
        int temp = 0;
        time_t t;
        srand((unsigned) time(&t));
        for (i=0; i<num_pieces; i++)    /* first filling the integer array serially */
                piece_index[i] = i;


        for (i=0; i<(num_pieces-1); i++) {
            r = i + (rand() % (num_pieces-i)); /* shuffling the integer array containg piece_index */
            temp = piece_index[i];
                piece_index[i] = piece_index[r];
                piece_index[r] = temp;
        }
}
  
int get_bitfield(bt_args_t * bt_args, bt_bitfield_t * bitfield_var)
{
        int i = 0;
        int x = bt_args->bt_info->num_pieces/8;
        int temp = 0;
        bitfield_var->size = bt_args->bt_info->num_pieces;

        if (x == 0)
                x++;
        else if (bt_args->bt_info->num_pieces%8 != 0)
                x++;

        temp = x*8;
        bitfield_var->bitfield = (unsigned char*)malloc(temp+1);
        memset(bitfield_var->bitfield,0,temp+1);

        for (i=0; i<(bt_args->bt_info->num_pieces); i++) {
                bitfield_var->bitfield[i] = '1';
        }
        for (i=(bt_args->bt_info->num_pieces); i<temp; i++)
                bitfield_var->bitfield[i] = '0';

                bitfield_var->bitfield[temp] = '\0';
        printf("\nBitfield message sent = %s\n", bitfield_var->bitfield);
        fflush(stdout);


        return 0;
}

void create_bitfield_message(bt_bitfield_t * bitfield_var, unsigned char * message)
{

        int var = 0;
        unsigned char * temp = message;
        memset(message, 0, MESSAGE_SIZE);
        var = (int)bitfield_var->size + 1;
        snprintf((char*)temp,MESSAGE_SIZE,"%s%u#%s%u#%s%s$%s","length:",var,"id:",BT_BITFILED,"bitfield:",bitfield_var->bitfield,"\0");

}
/* here we populate the bitfield structure on leecher and print the received bitfield */
void receive_bitfield_message(bt_bitfield_t * bitfield_var, unsigned char * message)
{
        unsigned char * temp = message;
        int integer_read = 0;
        temp = temp + strlen("length:");
        sscanf((char*)temp,"%d",&integer_read); /* reading field length */
        bitfield_var->size = integer_read-1;
        temp = (unsigned char*)strstr((char*)message,"bitfield:");
        temp = temp+strlen("bitfield:");
        integer_read = (int)strlen((char*)temp);
        integer_read--;
        bitfield_var->bitfield = (unsigned char*)malloc(integer_read+1);
        memset(bitfield_var->bitfield,0,integer_read+1);
        strncpy((char*)bitfield_var->bitfield,(char*)temp,integer_read);
        bitfield_var->bitfield[integer_read] = '\0';
        printf("\nReceived bitfield message on leecher = %s\n",bitfield_var->bitfield);

}

void create_message(unsigned char* message, int message_type)
{
        int length = 1;
        memset(message,0,MESSAGE_SIZE);
        unsigned char * temp = message;
        memset(message, 0, MESSAGE_SIZE);
        snprintf((char*)temp,MESSAGE_SIZE,"%s%u#%s%u$%s","length:",length,"id:",message_type,"\0");

        if ( message_type == BT_INTERESTED )
                printf("\nMessage sent    : type [%d]: Interested Message\n", message_type);
        else if ( message_type == BT_UNCHOKE )
                printf("\nMessage sent    : type [%d]: Unchoke Message\n", message_type);

}

void create_have_message(unsigned char* message, int message_type, int index)
{
        memset(message,0,MESSAGE_SIZE);
        unsigned char * temp = message;
        memset(message, 0, MESSAGE_SIZE);
        int length = 1+ sizeof(index);;
        snprintf((char*)temp,MESSAGE_SIZE,"%s%u#%s%u#%s%u$%s","length:",length,"id:",message_type,"index:",index,"\0");

}

int receive_message(unsigned char* message)
{
        int message_type = 0;
        unsigned char* temp = message;
        temp = (unsigned char*)strstr((char*)message,"id:");
        temp += strlen("id:");
        sscanf((char*)temp,"%d",&message_type);         /* reading message type */

        if ( message_type == BT_INTERESTED )
                printf("\nMessage received: type [%d]: Interested Message\n", message_type);
        else if ( message_type == BT_UNCHOKE )
                printf("\nMessage received: type [%d]: Unchoke Message\n", message_type);

        return (message_type);

}

void create_request_message(bt_request_t * req_msg, unsigned char* message, int index, int offset)
{
        int message_type = BT_REQUEST;
        unsigned char* temp = message;
        req_msg->index = index;
        req_msg->begin = offset;
        req_msg->length = BLOCK_SIZE;
        int length = 1+ sizeof(index) + sizeof(offset) + sizeof(req_msg->length);
        memset(message,0,MESSAGE_SIZE);
        snprintf((char*)temp,MESSAGE_SIZE,"%s%u#%s%u#%s%u#%s%u#%s%u$%s","length:",length,"id:",message_type,"index:",req_msg->index,"begin:",req_msg->begin,"block:",req_msg->length,"\0");

        fflush(stdout);

}

void receive_request_message(bt_request_t * req_msg, unsigned char* message)
{
        int index = 0;
        int offset = 0;
        int message_type = 0;
        int blck_size = 0;
        unsigned char* temp = message;
        temp = (unsigned char*)strstr((char*)message,"id:");
        temp += strlen("id:");
        sscanf((char*)temp,"%d",&message_type);         /* reading message type */
        
        temp = (unsigned char*)strstr((char*)message,"index:");
        temp += strlen("index:");
        sscanf((char*)temp,"%d",&index);                /* reading piece index  */

        temp = (unsigned char*)strstr((char*)message,"begin:");
        temp += strlen("begin:");
        sscanf((char*)temp,"%d",&offset);               /* reading offset  */

        temp = (unsigned char*)strstr((char*)message,"block:");
        temp += strlen("block:");
        sscanf((char*)temp,"%d",&blck_size);            /* reading block size */

        req_msg->index = index;
        req_msg->begin = offset;
        req_msg->length = blck_size;

}

void create_piece_message(bt_request_t * req_msg, unsigned char* message, unsigned char* temp_buffer, int fileread)
{
        int message_type = BT_PIECE;
        unsigned char* temp = message;
        int length = 1+ sizeof(req_msg->begin) + sizeof(req_msg->index)+ (int)strlen((char*)temp_buffer);
        memset(message,0,PIECE_BUFFER_SIZE);
        if(fileread ==0) {
                strcpy((char*)temp, "stop\0");
                return;
        }
        snprintf((char*)temp,PIECE_BUFFER_SIZE,"%s%u#%s%u#%s%u#%s%u#%s%s$%s","length:",length,"id:",message_type,"index:",req_msg->index,"begin:",req_msg->begin,"data:",temp_buffer,"\0");

}

void extract_block_from_message(unsigned char* message,unsigned char* temp_buffer, int * index, int * offset)
{
        unsigned char* temp = message;
        temp = (unsigned char*)strstr((char*)message,"data:");
        temp += strlen("data:");
        memset(temp_buffer,0, PIECE_BUFFER_SIZE);
        strncpy((char*)temp_buffer, (char*)temp, BLOCK_SIZE);

        temp = message;
        temp = (unsigned char*)strstr((char*)message,"index:");
        temp += strlen("index:");
        sscanf((char*)temp,"%d",index); /* reading piece index from piece message */


        temp = message;
        temp = (unsigned char*)strstr((char*)message,"begin:");
        temp += strlen("begin:");
        sscanf((char*)temp,"%d",offset);        /* reading block offset from piece message */


}



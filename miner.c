#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include "headers/JSON.h"
#include "headers/config.h"
#include "headers/user_options.h"
#include "headers/http.h"

struct transaction {
	unsigned char transaction_signature[64];
	unsigned char signing_public_key[32]; /* a public key */
	unsigned long timestamp;
	unsigned char destination_public_wallet_address[25];
	unsigned char sender_public_wallet_address[25];
	unsigned long transaction_amount;
	unsigned long transaction_fee;
	unsigned char is_transaction_fee;
};

struct block {
	unsigned int this_block_id;
	unsigned long current_timestamp;
	unsigned int difficulty_target;
	struct transaction transactions[25000];
	unsigned char merkle_root_of_transactions[32];
	unsigned char previous_sha256_block_hash[32];
	unsigned char nonce[32];
};



int main(int argc,char *argv[])
{

	/* Initialize variables */
	unsigned char my_public_key[32] = {0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
		0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
		0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05};
	unsigned char my_private_key[64] = {0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
		0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
		0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
		0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
		0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05};
	//char node_ip_address[40] = "192.168.1.144"; /* ascii representation of ipv6 is maximum 39 bytes */
	//char node_ip_address[40] = "142.251.163.101"; /* ascii representation of ipv6 is maximum 39 bytes */
	//char node_ip_address[NODE_IP_ADDRESS_SIZE + 1] = "192.168.1.145"; /* ascii representation of ipv6 is maximum 39 bytes + null character */
	char node_ip_address[NODE_IP_ADDRESS_SIZE + 3] = {0}; /* ascii representation of ipv6 is maximum 39 bytes + null character */
	char *http_request_browse;
	int retval_int;
	ssize_t retval_ssize_t;
	unsigned char retval_uchar;
	int main_socket;
	unsigned long all_pdn_earnings;
	unsigned int next_block; /* also called chainLength + 1 */
	unsigned char previous_sha256_block_hash[32]; /* needs conversion from node */
	unsigned int difficulty_target; /* also called challengeSize */
	unsigned long last_timestamp; /* need to convert from string */
	unsigned long mining_fee; /* also called 'total', the reward we get from solving block  */
	struct transaction fee_to_our_wallet; /* our reward :) */
	struct addrinfo *address;
	struct addrinfo hints;
	char port[PORT_SIZE + 3] = {0};
	char http_request[100] = "GET /mine HTTP/1.1\r\nHost: ";
	char *content_length = "\r\nContent-Length: 0\r\n\r\n";
	char *JSON_location;
	size_t string_len = 0;
	size_t http_request_len = 0;
	char http_response[HTTP_RESPONSE_SIZE + 1] = {0}; /* 10MB */
	unsigned long key_index;
	int i;

	/* Intro */
	printf("\n     _.-=-._.-=-._.-= MINERZ 0.01 =-._.-=-._.-=-._\n\n\n\n");

	get_user_options(argc,argv,port,sizeof(port),node_ip_address,sizeof(node_ip_address));

	/* Exit out of most incompatible memory architectures */
	if(sizeof(void *) != 8) {
		fprintf(stderr,"Error: please use a 64-bit computer\n");
		exit(1);
	}
	if(sizeof(int) != 4) {
		fprintf(stderr,"Error: please use a computer where an int is 4 bytes\n");
		exit(1);
	}
	if(sizeof(unsigned long) != 8) {
		fprintf(stderr,"Error: please use a computer where unsigned long is 8 bytes\n");
		exit(1);
	}

	/* Connect to node */
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(node_ip_address, port, &hints, &address) != 0) {
		perror("Error, getaddrinfo() failed");
		exit(1);
	}
	if(address->ai_family != AF_INET && address->ai_family != AF_INET6) {
		fprintf(stderr,"Error, server address isn't ipv4 or ipv6, exiting...\n");
		exit(1);
	}
	if((main_socket = socket(address->ai_family,SOCK_STREAM,0)) == -1) {
		perror("Error, socket() failed");
		exit(1);
	}
	printf("Connecting to Pandanite node at address %s, port %s...\n",node_ip_address,port);
	if(connect(main_socket,address->ai_addr,address->ai_addrlen) == -1) {
		perror("Error, connect() failed");
		exit(1);
	}
	printf("Success, Connected to Pandanite node at address %s, port %s...\n",node_ip_address, port);
	freeaddrinfo(address);

	http_request_browse = http_request + strlen(http_request);
	string_len = strlen(node_ip_address);
	memcpy(http_request_browse,node_ip_address,string_len);
	http_request_browse += string_len;
	memcpy(http_request_browse,":",1);
	http_request_browse++;
	string_len = strlen(port);
	memcpy(http_request_browse,port,string_len);
	http_request_browse += string_len;
	string_len = strlen(content_length);
	memcpy(http_request_browse,content_length,string_len + 1); /* extra 1 for null character */
	http_request_browse += string_len;
	http_request_len = http_request_browse - http_request;

	struct block block_to_hash;

	/* mining loop */
	for(;;) {

		/* ask node for mining details via /mine http request to node API */
		retval_ssize_t = write(main_socket,http_request,http_request_len);
		if(retval_ssize_t == -1) {
			perror("Error, write() failed");
			exit(1);
		} else if (retval_ssize_t != http_request_len) {
			fprintf(stderr,"partial write occurred," 
					" need to write code to handle this later\nexiting...\n");

			exit(1);
		}
		printf("HTTP request: \n\n%s\n",http_request);
		printf("request length: %zu\n",strlen(http_request));

		/* receive mining details from node API into http_response buf */
		http_recv(main_socket,http_response,&JSON_location);
		printf("\nHTTP response: \n\n%s\n\n",http_response);

		/* Parse body of http response from http_response buf
		 * The body should be a JSON object containing the mining details */
		if(parse_JSON(&JSON_location) == FAILURE) {
			fprintf(stderr,"Could not parse JSON sent from node server\n");
			exit(EXIT_FAILURE);
		}

		/* Look for mining variables in JSON sent from node via HTTP response
		 * The parsed JSON variables we are looking through were stored in 'first_JSON_object'
		 * by the previous parse_JSON() function we called.
		 * 'first_JSON_object' is a global array of struct member.
		 * The array was made global via 'extern' in JSON.h */
		printf("Mining details: \n");
		key_index = find_key_index("chainLength",IS_NUMBER);
		next_block = (unsigned int) first_JSON_object[key_index].data.number_bool_or_null_type + 1;
		printf("next block: %d\n",next_block);
		key_index = find_key_index("challengeSize",IS_NUMBER);
		difficulty_target = (unsigned int) first_JSON_object[key_index].data.number_bool_or_null_type;
		printf("difficulty target: %d\n",difficulty_target);
		key_index = find_key_index("lastHash",IS_STRING);
		printf("last block hash: %s\n",first_JSON_object[key_index].data.string_type);
		if(strlen(first_JSON_object[key_index].data.string_type) != 64) {
			fprintf(stderr,"last block hash sent from node is not 32 bytes\n");
		}
		key_index = find_key_index("lastTimestamp",IS_STRING);
		printf("Last timestamp: %s\n",first_JSON_object[key_index].data.string_type);
		key_index = find_key_index("miningFee",IS_NUMBER);
		mining_fee = first_JSON_object[key_index].data.number_bool_or_null_type;
		printf("Mining fee aka block reward: %lu\n",first_JSON_object[key_index].data.number_bool_or_null_type);

		printf("\n");
		printf("\nmining...\n\n");
		usleep(1000000); /* 1 second currently */

	}

	/* then close socket*/
	close(main_socket);


	

	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "headers/JSON.h"
#include "headers/config.h"
#include "headers/user_options.h"
#include "headers/http.h"
#include "headers/utils.h"


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
	unsigned char previous_sha256_block_hash[32]; /* important to be unsigned, or else a function breaks */
	unsigned char nonce[32];
};



int
main(int argc,char *argv[])
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
	unsigned long current_timestamp;
	unsigned long mining_fee; /* also called 'total', the reward we get from solving block  */
	struct transaction fee_to_our_wallet; /* our reward :) */
	char port[PORT_SIZE + 3] = {0};
	char mine_request_headers[] = "GET /mine HTTP/1.1\r\nHost: ";
	char tx_request_headers[] = "GET /gettx HTTP/1.1\r\nContent-Type: application/octet-stream\r\nHost: ";
	char http_mine_request[200] = {0};
	char http_tx_request[300] = {0};
	char *content_length = "\r\nContent-Length: 0\r\n\r\n";
	char *end_ptr;
	char *JSON_location;
	char *tx_location;
	size_t string_len = 0;
	char http_response[HTTP_RESPONSE_SIZE + 1] = {0}; /* 10MB */
	unsigned long key_index;
	int i;

	/* Intro */
	printf("\n     _.-=-._.-=-._.-= MINERZ 0.01 =-._.-=-._.-=-._\n\n\n\n");

	/* Exit out of most incompatible memory architectures */
	if (sizeof(void *) != 8) {
		fprintf(stderr,"Error: please use a 64-bit computer\n");
		exit(EXIT_FAILURE);
	}
	if (sizeof(int) != 4) {
		fprintf(stderr,"Error: please use a computer where an int is 4 bytes\n");
		exit(EXIT_FAILURE);
	}
	if (sizeof(unsigned long) != 8) {
		fprintf(stderr,"Error: please use a computer where unsigned long is 8 bytes\n");
		exit(EXIT_FAILURE);
	}
	if (check_endianness() == IS_BIG_ENDIAN) {
		fprintf(stderr,"Error, please use a computer with little endian memory layout\n"
						"Your computer uses big endian memory layout instead\n");
		exit(EXIT_FAILURE);
	}

	/* Get user options */
	get_user_options(argc,argv,port,sizeof(port),node_ip_address,sizeof(node_ip_address));

	/* Connect to node */

	sprintf(http_mine_request,"%s%s:%s%s",mine_request_headers,node_ip_address,port,content_length);
	sprintf(http_tx_request,"%s%s:%s%s",tx_request_headers,node_ip_address,port,content_length);
	struct block block_to_hash;

	http_connect(&main_socket,node_ip_address,port);

	/* mining loop */
	for (;;) {

		/* ask node for mining details via /mine http request to node API */
		http_write(main_socket,http_mine_request);

		/* receive mining details from node API into http_response buf */
		http_recv(main_socket,http_response,&JSON_location);


		/* Parse body of http response from http_response buf
		 * The body should be a JSON object containing the mining details */
		if (parse_JSON(&JSON_location) == FAILURE) {
			fprintf(stderr,"Could not parse JSON sent from node server\n");
			exit(EXIT_FAILURE);
		}

		/* Look for mining variables in JSON sent from node via HTTP response
		 * The parsed JSON variables we are looking through were stored in 'first_JSON_object'
		 * by the previous parse_JSON() function we called.
		 * 'first_JSON_object' is a global array of struct member.
		 * The array was made global via 'extern' in JSON.h */
		printf("Mining details: \n");

		/* next block ID */
		key_index = find_key_index("chainLength",IS_NUMBER);
		if (first_JSON_object[key_index].data.number_bool_or_null_type + 1 > UINT_MAX) {
			fprintf(stderr,"next block ID cannot be larger than %u",UINT_MAX);
			exit(EXIT_FAILURE);
		}
		next_block = (unsigned int) first_JSON_object[key_index].data.number_bool_or_null_type + 1;
		printf("next block: %d\n",next_block);
		block_to_hash.this_block_id = next_block;

		/* difficulty target */
		key_index = find_key_index("challengeSize",IS_NUMBER);
		if (first_JSON_object[key_index].data.number_bool_or_null_type > UINT_MAX) {
			fprintf(stderr,"Challenge size cannot be larger than %u",UINT_MAX);
			exit(EXIT_FAILURE);
		}
		difficulty_target = (unsigned int) first_JSON_object[key_index].data.number_bool_or_null_type;
		printf("difficulty target: %d\n",difficulty_target);
		block_to_hash.difficulty_target = difficulty_target;

		/* last block hash */
		key_index = find_key_index("lastHash",IS_STRING);
		if (strlen(first_JSON_object[key_index].data.string_type) != 64) {
			fprintf(stderr,"last block hash sent from node is not 32 bytes\n");
		}
		null_character_terminated_64_byte_hex_string_to_32_bytes(first_JSON_object[key_index].data.string_type, previous_sha256_block_hash);
		printf("Last block hash: ");
		for(i = 0;i < 32; i++) {
			printf("%02x",previous_sha256_block_hash[i]);
		}
		printf("\n");
		memcpy(block_to_hash.previous_sha256_block_hash,previous_sha256_block_hash,32);

		/* last timestamp */
		key_index = find_key_index("lastTimestamp",IS_STRING);
		last_timestamp = strtoul(first_JSON_object[key_index].data.string_type,&end_ptr,10);
		if (end_ptr == first_JSON_object[key_index].data.string_type || last_timestamp == ULONG_MAX || *end_ptr != '\0') {
			fprintf(stderr,"invalid timestamp send from node server after /mine request\n");
			exit(EXIT_FAILURE);
		}
		printf("Last timestamp: %lu\n",last_timestamp);
		current_timestamp = (unsigned long) time(NULL);
		printf("Current timestamp: %lu\n",current_timestamp);
		if (current_timestamp < last_timestamp) {
			block_to_hash.current_timestamp = last_timestamp + 1;
		} else {
			block_to_hash.current_timestamp = current_timestamp;
		}

		/* mining fee */
		key_index = find_key_index("miningFee",IS_NUMBER);
		mining_fee = first_JSON_object[key_index].data.number_bool_or_null_type;
		printf("Mining fee aka block reward: %lu\n",first_JSON_object[key_index].data.number_bool_or_null_type);

		/* ask node for transactions to put in next block
		http_write(main_socket,http_tx_request);

		http_recv(main_socket,http_response,&tx_location);
		printf("body length: %zu\n",strlen(tx_location));
		if (strlen(tx_location) != 0) {
			printf("found transactions\n");
			exit(EXIT_SUCCESS);
		}
		*/

		printf("\n");
		printf("\nmining...\n\n");
		usleep(1000000); /* 1 second currently */

	}

	/* then close socket*/
	close(main_socket);

	return 0;
}

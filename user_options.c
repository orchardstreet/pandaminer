#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "headers/user_options.h"
#include "headers/readline_custom.h"
#include "headers/config.h"

unsigned char parse_command_line(int argc, char **argv, char *port, char *node_ip_address) {

	unsigned char cmd_port_valid = 0;
	unsigned char cmd_ip_valid = 0;
	int i;

	for(i=1;i < 4;i++) {
		if(!strcmp("-p",argv[i]) || !strcmp("-port",argv[i]) || !strcmp("--port",argv[i]) || !strcmp("--p",argv[i])) {
			if(strlen(argv[i+1]) > 0 && strlen(argv[i+1]) <= PORT_SIZE) {
				strcpy(port,argv[i+1]);
				cmd_port_valid = 1;
			} else {
				fprintf(stderr,"Entered an invalid number of characters for 'port'\n\n"
						"Let's try again...\n\n");
				return 0; /* invalid parameters */
			}
		}
		if(!strcmp("-ip",argv[i]) || !strcmp("--ip",argv[i])) {
			if(strlen(argv[i+1]) > 0 && strlen(argv[i+1]) <= NODE_IP_ADDRESS_SIZE) {
				strcpy(node_ip_address,argv[i+1]);
				cmd_ip_valid = 1;
			} else {
				fprintf(stderr,"Entered an invalid number of characters for 'ip address'\n\n"
						"Let's try again...\n\n");
				return 0; /* invalid parameters */
			}
		}
	}
	if(cmd_port_valid && cmd_ip_valid) {
		printf("Entered ip: %s\nEntered port: %s\n",node_ip_address,port);
		return 1; /* valid parameters s*/
	} else {
		fprintf(stderr,"Invalid command line parameters\n\nFormat: ./miner --ip [ip address] --port [port number]\n\n");
		return 0; /* invalid parameters */
	}

	return 0;

}

void get_user_options(int argc, char **argv, char *port, size_t port_buf_size, char *node_ip_address, size_t node_ip_address_buf_size) {

	size_t result_str_len;
	signed char retval;
	unsigned char has_valid_parameters = 0;

	if(argc > 1 && argc != 5) {
		fprintf(stderr,"Invalid command line parameters\n\nFormat: ./miner --ip [ip address] --port [port number]\n\n"
				"Let's try again...\n\n");
	} else if (argc == 5) {
		has_valid_parameters = parse_command_line(argc,argv,port,node_ip_address);
	}

	if(!has_valid_parameters) {
		retval = readline_custom("Enter the IP address of the node (q to quit): ",node_ip_address,node_ip_address_buf_size,&result_str_len);
		if(retval == EXIT_PROGRAM) {
			printf("Exiting program...\n");
			exit(EXIT_FAILURE);
		}
		printf("IP address entered: %s\n",node_ip_address);
		/* printf("result string length: %zu\n",result_str_len); */
		retval = readline_custom("Enter the port of the node (q to quit): ",port,port_buf_size,&result_str_len);
		if(retval == EXIT_PROGRAM) {
			printf("Exit readline and program...\n");
			exit(EXIT_FAILURE);
		}
		printf("Port entered: %s\n",node_ip_address);
		/* printf("result string length: %zu\n",result_str_len); */
	}

	return;
}

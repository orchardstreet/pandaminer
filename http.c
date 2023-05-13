#include <stdlib.h>
#include <stdio.h>
#include "headers/http.h"
#include "headers/config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>


unsigned char http_recv(int main_socket,char *http_response,char **body_location)
{
	int http_response_index = 0;
	int bytes_left_in_http_response = HTTP_RESPONSE_SIZE;
	ssize_t retval_ssize_t;
	unsigned char is_rn_present = 0;
	int content_length = 0;
	unsigned char found_content_length = 0;
	(*body_location) = NULL;


	for(;;) {
		retval_ssize_t = recv(main_socket,http_response + http_response_index,bytes_left_in_http_response,0);
		if(!retval_ssize_t) {
			printf("Connection closed by node server\n");
			return CONNECTION_CLOSED;
		} else if (retval_ssize_t == -1) {
			perror("Error, recv failed");
			exit(1);
		} else if (retval_ssize_t < 0) {
			fprintf(stderr,"Error, unspecified recv() error\n");
			exit(1);
		}
		http_response[http_response_index + retval_ssize_t] = 0;
		if(!(*body_location)) {
			(*body_location) =  strstr(http_response + http_response_index,"\r\n");
			(*body_location) += 2;
		}
		if((*body_location) && !found_content_length) {
			if(find_content_length(&content_length) == SUCCESS) {
				found_content_length = 1;
			} else {
				fprintf(stderr,"HTTP response from node missing 'content-length': %s",http_response);
				exit(EXIT_FAILURE);
			}
		}
		if(found_content_length && is_response_body_correct_length(*body_location) == SUCCESS) {
			return SUCCESS;
		}

		http_response_index += retval_ssize_t;
		bytes_left_in_http_response -= retval_ssize_t;
		if(!bytes_left_in_http_response) {
			fprintf(stderr,"HTTP response from node exceeded memory allocated\n"
				"printing HTTP response for debugging purposes: %s\n",http_response);
			exit(EXIT_FAILURE);
		}
		printf("\nHTTP response: %s\n\n",http_response);
	}
	/* is_rn_present(&http_response_browse); */

}

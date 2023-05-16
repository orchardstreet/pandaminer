#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <limits.h>
#include "headers/http.h"
#include "headers/config.h"

static unsigned char find_content_length(int *content_length, char *http_response) {

	char *browse_buf = http_response;
	unsigned long number;
	char *end_ptr;

	for(;;) {
		browse_buf = strstr(browse_buf,"\r\n");
		if (!browse_buf) {
			fprintf(stderr,"Unknown error parsing headers in HTTP response from node\n");
			exit(EXIT_FAILURE);
		}
		browse_buf += 2;
		if(!strncmp(browse_buf,"\r\n",2)) {
			fprintf(stderr,"HTTP response from node missing 'content-length': %s\n", http_response);
			exit(EXIT_FAILURE);
		}
		if(!strncmp(browse_buf,"Content-Length: ",16) || !strncmp(browse_buf,"content-length: ",16) || !strncmp(browse_buf,"Content-length: ",16)) {
			browse_buf += 16;
			number = strtoul(browse_buf,&end_ptr,10);
			if(end_ptr == browse_buf) {
				fprintf(stderr,"No number supplied as content-length in HTTP header\n");
				exit(EXIT_FAILURE);
			} else if(number == ULONG_MAX) {
				fprintf(stderr,"Number supplied as content-length in HTTP header is too large\n"
					"content-length must be under %lu\n",ULONG_MAX);
				exit(EXIT_FAILURE);
			}
			*content_length = number;
			return SUCCESS;
		}
	}

}

unsigned char http_recv(int main_socket,char *http_response,char **body_location)
{

	int http_response_index = 0;
	int bytes_left_in_http_response = HTTP_RESPONSE_SIZE;
	ssize_t retval_ssize_t;
	int content_length = 0;
	unsigned char found_content_length = 0;
	(*body_location) = NULL;

	memset(http_response,0,HTTP_RESPONSE_SIZE + 1);

	for(;;) {
		retval_ssize_t = recv(main_socket,http_response + http_response_index,bytes_left_in_http_response,0);
		if(!retval_ssize_t) {
			printf("Connection closed by node server\n");
			exit(EXIT_SUCCESS);
			/* return CONNECTION_CLOSED; */
		} else if (retval_ssize_t == -1) {
			perror("Error, recv failed");
			exit(EXIT_FAILURE);
		} else if (retval_ssize_t < 0) {
			fprintf(stderr,"Error, unspecified recv() error\n");
			exit(EXIT_FAILURE);
		}

		/* terminate response with null character */
		http_response[http_response_index + retval_ssize_t] = 0;
		/* printf("\nHTTP response: %s\n\n",http_response); */

		if(!(*body_location)) {
			(*body_location) =  strstr(http_response + http_response_index,"\r\n\r\n");
			if(*body_location) {
				(*body_location) += 4; /* move pointer to start of body, past \r\n\r\n */
			}
		}
		if((*body_location) && !found_content_length) {
			if(find_content_length(&content_length,http_response) == SUCCESS) {
				found_content_length = 1;
			} else {
				fprintf(stderr,"HTTP response from node missing 'content-length': %s\n",http_response);
				exit(EXIT_FAILURE);
			}
		}
		if(found_content_length && (strlen(*body_location) == content_length)) {
			return SUCCESS;
		}

		http_response_index += retval_ssize_t;
		bytes_left_in_http_response -= retval_ssize_t;

		if(!bytes_left_in_http_response) {
			fprintf(stderr,"HTTP response from node exceeded memory allocated\n"
				"printing HTTP response for debugging purposes: %s\n",http_response);
			exit(EXIT_FAILURE);
		}

	}

}

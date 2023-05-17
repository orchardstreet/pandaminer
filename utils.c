#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "headers/config.h"
#include "headers/utils.h"


unsigned char null_character_terminated_64_byte_hex_string_to_32_bytes(char *bytes_64_plus_null_orig, unsigned char *bytes_32)
{

	char *endptr;
	/*unsigned char bytes_32[32] = {0}; */
	unsigned char *bytes_32_browse = bytes_32 + 24;
	char bytes_64_plus_null[65] = {0};
	char *bytes_64_plus_null_browse;
	unsigned long temp_num;
	int i;

	memcpy(bytes_64_plus_null,bytes_64_plus_null_orig,65);
	bytes_64_plus_null_browse = bytes_64_plus_null + 48;

	for(;;bytes_32_browse -= 8,bytes_64_plus_null_browse -= 16) {
		temp_num = strtoul(bytes_64_plus_null_browse,&endptr,16);
		if(bytes_64_plus_null_browse == endptr) {
			fprintf(stderr,"Previous hash sent from node is invalid\n");
			exit(EXIT_FAILURE);
		}
		bytes_32_browse[0] = (temp_num >> 56) & 0xFF;
		bytes_32_browse[1] = (temp_num >> 48) & 0xFF;
		bytes_32_browse[2] = (temp_num >> 40) & 0xFF;
		bytes_32_browse[3] = (temp_num >> 32) & 0xFF;
		bytes_32_browse[4] = (temp_num >> 24) & 0xFF;
		bytes_32_browse[5] = (temp_num >> 16) & 0xFF;
		bytes_32_browse[6] = (temp_num >> 8) & 0xFF;
		bytes_32_browse[7] = temp_num & 0xFF;
		if(bytes_64_plus_null_browse == bytes_64_plus_null) {
			break;
		}
		*bytes_64_plus_null_browse = 0;
	}

	return SUCCESS;

}

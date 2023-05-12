#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "headers/JSON.h"
#include "headers/config.h"
/* RFC series (ISSN 2070-1721) STD 90, RFC 8259: https://datatracker.ietf.org/doc/html/rfc8259 */
/* ECMA-404, 2nd edition, December 2017: https://www.ecma-international.org/wp-content/uploads/ECMA-404_2nd_edition_december_2017.pdf */

struct member first_JSON_object[FIRST_JSON_OBJECT_SIZE];
unsigned long first_JSON_object_index = 0;

struct member spare_object_or_array_members[SPARE_OBJECT_OR_ARRAY_MEMBERS_SIZE];
unsigned long spare_object_or_array_members_index = 0;

char spare_chars[SPARE_CHARS_SIZE];
unsigned long spare_chars_index = 0;
unsigned char spare_chars_full = 0;


/* input string can't be longer than an int in length */
/* must be null character terminated */
/* will not fail on null character, check that after running if needed */
/* which means you should TODO check that null character is checked after running this each time */
unsigned char skip_whitespace_and_check_null(char **browse_string) {
	int i;
	for(i = 0;i < MAX_WHITESPACE_AROUND_STRUCTURAL_CHARACTER && (**browse_string == 0x20 || **browse_string == 0x09 || **browse_string == 0x0A || **browse_string == 0x0D);(*browse_string)++,i++) { 
		 /* printf("whitespace encountered, i: %d\n",i); */
	}	
	if(i==100) {
		fprintf(stderr,"Error, JSON has too much whitespace around a structural characater\n");
		return FAILURE;
	}

	if(!(**browse_string)) {
		fprintf(stderr,"Error, null character found before last closing bracket\n");
		return FAILURE;
	}

	return SUCCESS;
}

/* must be null character terminated */
unsigned char parse_JSON_object(char **browse_string) 
{
	unsigned char retval;
	if(skip_whitespace_and_check_null(browse_string) == FAILURE)
		return FAILURE;
	
	return SUCCESS;
}

unsigned char parse_string(char **browse_string)
{
	char *beginning_of_key;
	char *end_of_key;
	unsigned long valid_characters = 0;

	if(spare_chars_full) {
		fprintf(stderr,"Spare characters memory full, cannot parse further JSON,\n"
				"raise SPARE_CHARS_SIZE in config.h to increase available memory to parse"
				"the JSON\n");
		return FAILURE;
	}
	beginning_of_key = *browse_string;
	for(;;(*browse_string)++,valid_characters++) {
		if(valid_characters == ULONG_MAX) {
			fprintf(stderr,"Error, A continuous string in JSON body cannot exceed %lu characters\n",ULONG_MAX);
			return FAILURE;
		}
		if(!(**browse_string)) {
			fprintf(stderr,"Error, null character found before last closing bracket\n");
			return FAILURE;
		}
		/*TODO check for escape characters by creating parse_escape_sequence() function */
		/*
		if(**browse_string == '\') { if(parse_escape_sequence(browse_string) == FAILURE)
				return FAILURE;

		}
		*/
		if(**browse_string == '"') {
			end_of_key = *browse_string;
			(*browse_string)++;
			/* if key is an empty string */
			if(end_of_key == beginning_of_key) {
				spare_chars[spare_chars_index] = 0;
				/* store a null character in spare_chars to represent an empty string */
				first_JSON_object[first_JSON_object_index].data_type = IS_STRING;
				first_JSON_object[first_JSON_object_index].data.string_type = spare_chars + spare_chars_index;
				spare_chars_index++;
				if(spare_chars_index == SPARE_CHARS_SIZE - 1) {
					spare_chars_full = 1;
				}
				return SUCCESS;
			} else {
				/* if key is a non-empty string and there's enough memory to store the string in spare_chars buffer
				 * including a null character appended at the end of the string */
				if(valid_characters + 1 <= SPARE_CHARS_SIZE - spare_chars_index) {
					/* store the key in spare_chars */
					memcpy(spare_chars + spare_chars_index, beginning_of_key, valid_characters);
					first_JSON_object[first_JSON_object_index].data_type = IS_STRING;
					first_JSON_object[first_JSON_object_index].data.string_type = spare_chars + spare_chars_index;
					spare_chars_index += valid_characters;
					spare_chars[spare_chars_index] = 0; /* null terminate key in spare_chars array */
					/* if we hit the spare_chars memory limit, but didn't exceed it, tell program not to iterate
					 * any variables or append to any buffers associated with the spare_chars array */
					if(spare_chars_index == SPARE_CHARS_SIZE - 1) {
						spare_chars_full = 1;
					} else {
						spare_chars_index++; /* move to empty position */
					}
					return SUCCESS;
				/* if out of memory to store keyname, return parse as FAILURE, and exit parsing */
				} else {
					fprintf(stderr,"Not enough memory to store string\n");
					return FAILURE;
				}
			}
		}
	}
}
unsigned char validate_key(char **browse_string)
{
	char *beginning_of_key;
	char *end_of_key;
	unsigned long valid_characters = 0;

	if(spare_chars_full) {
		fprintf(stderr,"Spare characters memory full, cannot parse further JSON,\n"
				"raise SPARE_CHARS_SIZE in config.h to increase available memory to parse"
				"the JSON\n");
		return FAILURE;
	}
	if(**browse_string != '"') {
		fprintf(stderr,"Key is missing first quotation mark\n");
		return FAILURE;
	}
	(*browse_string)++;
	beginning_of_key = *browse_string;
	for(;;(*browse_string)++,valid_characters++) {
		if(valid_characters == ULONG_MAX) {
			fprintf(stderr,"Error, A continuous string in JSON body cannot exceed %lu characters\n",ULONG_MAX);
			return FAILURE;
		}
		if(!(**browse_string)) {
			fprintf(stderr,"Error, null character found before last closing bracket\n");
			return FAILURE;
		}
		/*TODO check for escape characters by creating parse_escape_sequence() function */
		/*
		if(**browse_string == '\') { if(parse_escape_sequence(browse_string) == FAILURE)
				return FAILURE;

		}
		*/
		if(**browse_string == '"') {
			end_of_key = *browse_string;
			(*browse_string)++;
			if(skip_whitespace_and_check_null(browse_string) == FAILURE)
				return FAILURE;
			if(**browse_string != ':') {
				fprintf(stderr,"Error, second unescaped double quotation mark in key string"
					       " is not followed by a colon\n");
				return FAILURE;
			}
			(*browse_string)++;
			if(skip_whitespace_and_check_null(browse_string) == FAILURE)
				return FAILURE;
			/* if key is an empty string */
			if(end_of_key == beginning_of_key) {
				spare_chars[spare_chars_index] = 0;
				/* store a null character in spare_chars to represent an empty string */
				first_JSON_object[first_JSON_object_index].key = spare_chars + spare_chars_index;
				spare_chars_index++;
				if(spare_chars_index == SPARE_CHARS_SIZE - 1) {
					spare_chars_full = 1;
				}
				return SUCCESS;
			} else {
				/* if key is a non-empty string and there's enough memory to store the string in spare_chars buffer
				 * including a null character appended at the end of the string */
				if(valid_characters + 1 <= SPARE_CHARS_SIZE - spare_chars_index) {
					/* store the key in spare_chars */
					memcpy(spare_chars + spare_chars_index, beginning_of_key, valid_characters);
					first_JSON_object[first_JSON_object_index].key = spare_chars + spare_chars_index;
					spare_chars_index += valid_characters;
					spare_chars[spare_chars_index] = 0; /* null terminate key in spare_chars array */
					/* if we hit the spare_chars memory limit, but didn't exceed it, tell program not to iterate
					 * any variables or append to any buffers associated with the spare_chars array */
					if(spare_chars_index == SPARE_CHARS_SIZE - 1) {
						spare_chars_full = 1;
					} else {
						spare_chars_index++; /* move to empty position */
					}
					return SUCCESS;
				/* if out of memory to store keyname, return parse as FAILURE, and exit parsing */
				} else {
					fprintf(stderr,"Not enough memory to store JSON key name\n");
					return FAILURE;
				}
			}
		}
	}
}

unsigned char parse_true(char **browse_string) 
{
	if(!strncmp(*browse_string,"rue",3)) {
		(*browse_string) += 3;
		first_JSON_object[first_JSON_object_index].data_type = IS_BOOL;
		first_JSON_object[first_JSON_object_index].data.number_bool_or_null_type = 1;
		return SUCCESS;
	}

	fprintf(stderr,"Invalid JSON member value, perhaps you meant 'true'?\n");
	return FAILURE;

}

unsigned char parse_false(char **browse_string) 
{
	if(!strncmp(*browse_string,"alse",4)) {
		(*browse_string) += 4;
		first_JSON_object[first_JSON_object_index].data_type = IS_BOOL;
		first_JSON_object[first_JSON_object_index].data.number_bool_or_null_type = 1;
		return SUCCESS;
	}

	fprintf(stderr,"Invalid JSON member value, perhaps you meant 'false'?\n");
	return FAILURE;

}

unsigned char parse_null(char **browse_string) 
{
	if(!strncmp(*browse_string,"ull",3)) {
		(*browse_string) += 3;
		first_JSON_object[first_JSON_object_index].data_type = IS_BOOL;
		first_JSON_object[first_JSON_object_index].data.number_bool_or_null_type = 0;
		return SUCCESS;
	}

	fprintf(stderr,"Invalid JSON member value, perhaps you meant 'null'?\n");
	return FAILURE;
}

unsigned char parse_array(char **browse_string)
{
	fprintf(stderr,"object parsing not implemented yet\n");
	return FAILURE;
}

unsigned char parse_object(char **browse_string) 
{
	fprintf(stderr,"object parsing not implemented yet\n");
	return FAILURE;
}

unsigned char parse_number(char **browse_string) 
{
	char *the_endptr;
	unsigned long strtoul_result = 0;
	/* handle a leading '0' correctly because strtoul ignores it, and only '0' representing 0 is ok
	 * and leading '0's are not ok */
	if(**browse_string == '0') {
		(*browse_string)++;
		first_JSON_object[first_JSON_object_index].data_type = IS_NUMBER;
		first_JSON_object[first_JSON_object_index].data.number_bool_or_null_type = 0;
		return SUCCESS;
	}
	strtoul_result = strtoul(*browse_string,&the_endptr,10);
	if(strtoul_result == ULONG_MAX) {
		fprintf(stderr,"Cannot parse a number equal to or larger than %lu\n",ULONG_MAX);
		return FAILURE;
	} else if(the_endptr == *browse_string) {
		fprintf(stderr,"Unknown error parsing number\n");
		return FAILURE;
	}
	*browse_string = the_endptr;
	first_JSON_object[first_JSON_object_index].data_type = IS_NUMBER;
	first_JSON_object[first_JSON_object_index].data.number_bool_or_null_type = strtoul_result;
	return SUCCESS;
}

unsigned char parse_JSON(char **browse_string) 
{
	unsigned char value_type;
	/* skip whitespace and check for null character */
	if(skip_whitespace_and_check_null(browse_string) == FAILURE)
		return FAILURE;
	/* Check for opening bracket */
	if(**browse_string != '{') { 
		fprintf(stderr,"Error, JSON doesn't start with opening bracket '{'\n");
		return FAILURE;
	}
	/* iterate string */
	(*browse_string)++;
	/* skip whitespace and check for null character */
	if(skip_whitespace_and_check_null(browse_string) == FAILURE)
		return FAILURE;
	/* check for closing bracket */ 
	if(**browse_string == '}')
		return SUCCESS;


	for(;;) {


		/* parse JSON key */
		if(validate_key(browse_string) == FAILURE)
			return FAILURE;

		/* parse JSON member value */
		if(!(**browse_string)) {
			fprintf(stderr,"Error, null character found before last closing bracket\n");
			return FAILURE;
		} else if (**browse_string == 't') {
			(*browse_string)++;
			if(parse_true(browse_string) == FAILURE)
				return FAILURE;
		} else if ( **browse_string == 'f') {
			(*browse_string)++;
			if(parse_false(browse_string) == FAILURE)
				return FAILURE;
		} else if (**browse_string == 'n') {
			(*browse_string)++;
			if(parse_null(browse_string) == FAILURE)
				return FAILURE;
		} else if (**browse_string == '[') {
			(*browse_string)++;
			if(parse_array(browse_string) == FAILURE)
				return FAILURE;
		} else if (**browse_string == '{') {
			(*browse_string)++;
			if(parse_object(browse_string) == FAILURE)
				return FAILURE;
		} else if (**browse_string >= '0' && **browse_string <= '9') {
			if(parse_number(browse_string) == FAILURE)
				return FAILURE;
		} else if (**browse_string == '"') {
			(*browse_string)++;
			if(parse_string(browse_string) == FAILURE)
				return FAILURE;
		} else {
			fprintf(stderr,"%c is an invalid character to start a JSON member value with \n",**browse_string);
			return FAILURE;
		}

		/* skip whitespace and check for null character */
		if(skip_whitespace_and_check_null(browse_string) == FAILURE)
			return FAILURE;

		/* validate structural character after key/value pair */
		if(**browse_string == '}') {
			return SUCCESS;
		} else if (**browse_string == ',') {
			first_JSON_object_index++;
			if(first_JSON_object_index == FIRST_JSON_OBJECT_SIZE - 1 || first_JSON_object_index == ULONG_MAX - 1) {
				fprintf(stderr,"JSON has too many non-nested members\n"
						"must be under %d members",FIRST_JSON_OBJECT_SIZE);
				return FAILURE;
			}
			(*browse_string)++;
			if(skip_whitespace_and_check_null(browse_string) == FAILURE)
				return FAILURE;
			continue;
		} else {
			fprintf(stderr,"%c is an invalid structural character\n",**browse_string);
			return FAILURE;
		}

	}
	

	/* return SUCCESS; */
}

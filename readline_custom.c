#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "headers/readline_custom.h"
#include "headers/config.h"
#define QUIT_PROMPT_RESULT_SIZE 6

static char quit_prompt_input[QUIT_PROMPT_RESULT_SIZE]; /* Every compiler 0s out global variables, so this is 0d out, doesn't matter for current implementation though */

static signed char
flush_stdin(void)
{

	int c;
	for (;;) {
		c = getc(stdin);
		if(c == '\n') {
			return CONTINUE;
		} else if (c == EOF) {
			if(isatty(0))
				printf("\n");
			printf("detected EOF\n");
			clearerr(stdin);
			return EOF_SEEN;
		}
	}

}

static signed char
prompt_quit(char *prompt)
{
	/* Asks user if he wants to quit or not
         * returns YES on "yes", NO on "no, and EXIT_PROGRAM if the program should exit 
	 * Attempts to read from stdin to memory, until a valid input is given. 
	 * Asks user again on no valid input, no input, or on overflow 
	 * This program also flushes stdin on overflow. 
	 * This function deals with pecularities of stdin redirect input, in case
	 * someone is automating the program with stdin redirect. */
	
	char *newline_position;
	char *browse_input;
	signed char special_character_present = 0;
	signed char retval;
	
	for (;;) {
		
		printf("%s",prompt);	
		fflush(stdout);
		/* Using memset here in case there's a funky fgets implementation that errors out after
		 * detecting EOF and partially reads into buffer.  Or one that doesn't error
		 * out after detecting EOF, but also doesn't insert a null character.
		 * Also 0ing out the buffer before fgets helps with program analysis */
		memset(quit_prompt_input,0,QUIT_PROMPT_RESULT_SIZE); 
		if (fgets(quit_prompt_input,QUIT_PROMPT_RESULT_SIZE - 1,stdin) == NULL) {
			if (ferror(stdin)) {
				perror("");
				fprintf(stderr,"\nfgets failure\n");
				return EXIT_PROGRAM; 
			} else if (feof(stdin)) {
				printf("\ndetected EOF\n");
				clearerr(stdin);
				return EXIT_PROGRAM;
			} else {
				fprintf(stderr,"\nunhandled error\n");
				return EXIT_PROGRAM; /* exit */
			}
		}
		if (!isatty(0)) {
			printf("\n");
		}
		
		/* CHECK FOR OVERFLOW, this should be the first check in this function that doesn't exit entire program (besides EOF check) */
		newline_position = memchr(quit_prompt_input,'\n',QUIT_PROMPT_RESULT_SIZE - 2);
		if (!newline_position) {
			/* flush stdin */
			retval = flush_stdin();
			if (retval == CONTINUE) {
				printf("Cannot enter more than 3 characters, try again\n");
				continue;
			} else if (retval == EOF_SEEN) {
				return EXIT_PROGRAM;
			}
		}
		
		/* We didn't overflow stdin */

		/* If nothing entered, jump to top of function */
		if (newline_position == quit_prompt_input) {
			printf("Nothing entered, try again\n");
			continue;
		}

		/* Get rid of newline */
		*newline_position = 0;
		
		/* Check if fgets read a null character, in which case warn user of not entering special characters
		 * This null character check should always be here, so we don't process an empty string */
		/* Check for other special characters beyond first byte.  This check beyond the first byte can be modified  
		 * At this point in the function, a newline position exists, and it's not the first character */
		for (browse_input = quit_prompt_input;browse_input < newline_position ;browse_input++) {
			if (*browse_input < 'A' || (*browse_input < 'a' && *browse_input > 'Z') || *browse_input > 'z' ) {
				special_character_present = 1;
				break;
			}
		}
		
		if (special_character_present) {
			printf("Invalid input, please enter letters from a-z\n");
			special_character_present = 0;
			continue;
		} 

		if (!strcmp(quit_prompt_input,"no") || !strcmp(quit_prompt_input,"NO") || !strcmp(quit_prompt_input,"n") || !strcmp(quit_prompt_input,"N")) {
			return NO;
		} else if(!strcmp(quit_prompt_input,"yes") || !strcmp(quit_prompt_input,"YES") || !strcmp(quit_prompt_input,"Y") || !strcmp(quit_prompt_input,"y")) {
			return YES;
		} else {
			printf("Invalid input, please enter (y)es or (n)o\n");
			continue;
		}
		
	}
	
} 

signed char
readline_custom(char *prompt,char *input, size_t input_size_temp,size_t *result_string_length)
{
	/* !! Don't pass a pointer to an array of size less than 4 !! */
	/* Asks user for input.
	 * Attempts to read from stdin to memory, until a valid input is given.
	 * Asks user again on no input, on overflow, or if detects a special
	 * character.
	 * This program also flushes stdin on overflow.
	 * This function deals with pecularities of stdin redirect input, in case
	 * someone is automating the program with stdin redirect.
	 * RETURN VALUES: uses the exit_codes enum, which is self-explanatory */
	
	char *newline_position;
	char *browse_input;
	signed char special_character_present = 0;
	signed char retval;
	*result_string_length = 0;
	
	if (input == NULL) {
		fprintf(stderr,"passed NULL pointer to get_server_info_from_stdin() function\n");
		return EXIT_PROGRAM; /* exit */
	}
	
	if (input_size_temp < 4) {
		fprintf(stderr,"buffer provided not enough to read into fgets in function readline_custom\n");
		return EXIT_PROGRAM; /* exit */
	}
		
	for (;;) {
		
		printf("%s",prompt);
		fflush(stdout);
		
		/* Using memset here in case there's a funky fgets implementation that errors out after
		 * detecting EOF and partially reads into buffer.  Or one that doesn't error
		 * out after detecting EOF, but also doesn't insert a null character.
		 * Also 0ing out the buffer before fgets helps with program analysis */
		memset(input,0,input_size_temp);
		if (fgets(input,input_size_temp - 1,stdin) == NULL) {
			if (ferror(stdin)) {
				perror("");
				fprintf(stderr,"\nfgets failure\n");
				return EXIT_PROGRAM; 
			} else if (feof(stdin)) {
				printf("\ndetected EOF\n");
				clearerr(stdin);
				retval = prompt_quit("Are you sure you want to quit? (y)es/(n)o: ");
				if(retval == EXIT_PROGRAM || retval == YES)
					return EXIT_PROGRAM;
				continue;
			} else {
				fprintf(stderr,"\nunhandled error\n");
				return EXIT_PROGRAM; /* exit */
			}
		}
		if (!isatty(0)) {
			printf("\n");
		}
		
		/* CHECK FOR OVERFLOW, this should be the first check in this function that doesn't exit entire program (besides EOF check) */
		newline_position = memchr(input,'\n',input_size_temp - 2);
		if (!newline_position) {
			/* flush stdin */
			retval = flush_stdin();
			if (retval == CONTINUE) {
				printf("Cannot enter more than %lu characters, try again\n",(unsigned long) input_size_temp - 3);
				continue;
			} else if (retval == EOF_SEEN) {
				retval = prompt_quit("Are you sure you want to quit? (y)es/(n)o: ");
				if(retval == EXIT_PROGRAM || retval == YES)
					return EXIT_PROGRAM;
				continue;
			}
		}

		/* If nothing entered, jump to top of function */
		if (newline_position == input) {
			printf("Nothing entered, try again\n");
			continue;
		}

		/* Get rid of newline */
		*newline_position = 0;
		
		if (*input == 'q' && input[1] == 0) {
			retval = prompt_quit("Are you sure you want to quit? (y)es/(n)o: ");
			if(retval == EXIT_PROGRAM || retval == YES)
				return EXIT_PROGRAM;
			continue;
		}
		
		/* Check if fgets read a null character into first byte, in which case warn user of not entering special characters
		 * This null character check should always be here, so we don't process an empty string */
		/* Also, check for other special characters beyond the first byte.  Checking beyond first byte for null character
		 * can be modified.
		 * At this point in the function, a newline position exists, and it's not the first character */
		for (browse_input = input;browse_input < newline_position ;browse_input++) {
			if (*browse_input < ' ' ) {
				special_character_present = 1;
				break;
			}
		}
		
		if (special_character_present) {
			printf("Invalid input, please no special characters in input` \n");
			special_character_present = 0;
			continue;
		} else {
			break; /* leave entire function as SUCCESS */
		}
	}

	*result_string_length = newline_position - input;
	
	return SUCCESS; /* success */
} 

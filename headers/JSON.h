#ifndef JSON_H
#define JSON_H
#include "config.h"

enum data_types {IS_NUMBER,IS_BOOL,IS_NULL,IS_OBJECT,IS_ARRAY,IS_STRING};

union member_data {
        unsigned long number_bool_or_null_type;
        struct member *object_or_array_type;
        char *string_type;
};

struct member {
        char *key;
        unsigned char data_type;
        union member_data data;
};


unsigned char parse_JSON(char **browse_string);
unsigned long find_key_index(char *keyname,unsigned char the_data_type);

extern struct member first_JSON_object[FIRST_JSON_OBJECT_SIZE];
extern unsigned long first_JSON_object_index;
extern char spare_chars[SPARE_CHARS_SIZE];


#endif

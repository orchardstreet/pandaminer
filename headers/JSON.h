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

extern struct member first_JSON_object[FIRST_JSON_OBJECT_SIZE];

#endif

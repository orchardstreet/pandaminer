#ifndef UTILS_H
#define UTILS_H

enum endianness {IS_BIG_ENDIAN, IS_LITTLE_ENDIAN};

unsigned char null_character_terminated_64_byte_hex_string_to_32_bytes(char *bytes_64_plus_null_orig, unsigned char *bytes_32);
unsigned char check_endianness(void);

#endif

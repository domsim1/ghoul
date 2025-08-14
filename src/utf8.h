#ifndef ghoul_utf8_h
#define ghoul_utf8_h

#include <stdint.h>
#include <stdbool.h>

struct ObjString;

int utf8_char_length(unsigned char byte);
int utf8_string_length(const char* str, int byte_length);
bool utf8_is_valid(const char* str, int byte_length);
int utf8_char_at_index(const char* str, int char_index, int byte_length);
int utf8_next_char(const char* str, int current_byte_pos, int byte_length);

bool utf8_is_ascii_only(const char* str, int byte_length);
int utf8_string_length_fast(const char* str, int byte_length);

int utf8_get_cached_length(struct ObjString* string);
bool utf8_get_cached_is_ascii(struct ObjString* string);

#endif
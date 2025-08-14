#include "utf8.h"
#include "object.h"
#include <stddef.h>
int utf8_char_length(unsigned char byte) {
    if (byte < 0x80) return 1;
    if (byte < 0xC0) return -1;
    if (byte < 0xE0) return 2;
    if (byte < 0xF0) return 3;
    if (byte < 0xF8) return 4;
    return -1;
}

int utf8_string_length(const char* str, int byte_length) {
    if (str == NULL || byte_length < 0) {
        return 0;
    }
    if (byte_length == 0) {
        return 0;
    }
    
    int char_count = 0;
    int pos = 0;
    
    while (pos < byte_length) {
        int char_len = utf8_char_length((unsigned char)str[pos]);
        if (char_len <= 0) {
            char_count++;
            pos++;
            continue;
        }
        if (pos + char_len > byte_length) {
            char_count++;
            break;
        }
        
        bool valid_sequence = true;
        for (int i = 1; i < char_len; i++) {
            unsigned char cont_byte = (unsigned char)str[pos + i];
            if ((cont_byte & 0xC0) != 0x80) {
                valid_sequence = false;
                break;
            }
        }
        
        if (!valid_sequence) {
            char_count++;
            pos++;
            continue;
        }
        
        char_count++;
        pos += char_len;
    }
    
    return char_count;
}

bool utf8_is_valid(const char* str, int byte_length) {
    if (str == NULL) {
        return false;
    }
    if (byte_length < 0) {
        return false;
    }
    if (byte_length == 0) {
        return true;
    }
    
    int pos = 0;
    
    while (pos < byte_length) {
        unsigned char byte = (unsigned char)str[pos];
        int char_len = utf8_char_length(byte);
        
        if (char_len <= 0) {
            return false;
        }
        
        if (pos + char_len > byte_length) {
            return false;
        }
        
        for (int i = 1; i < char_len; i++) {
            unsigned char cont_byte = (unsigned char)str[pos + i];
            if ((cont_byte & 0xC0) != 0x80) {
                return false;
            }
        }
        
        if (char_len == 2) {
            unsigned int codepoint = ((byte & 0x1F) << 6) | (str[pos + 1] & 0x3F);
            if (codepoint < 0x80) {
                return false;
            }
        } else if (char_len == 3) {
            unsigned int codepoint = ((byte & 0x0F) << 12) | 
                                   ((str[pos + 1] & 0x3F) << 6) | 
                                   (str[pos + 2] & 0x3F);
            if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                return false;
            }
        } else if (char_len == 4) {
            unsigned int codepoint = ((byte & 0x07) << 18) | 
                                   ((str[pos + 1] & 0x3F) << 12) | 
                                   ((str[pos + 2] & 0x3F) << 6) | 
                                   (str[pos + 3] & 0x3F);
            if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
                return false;
            }
        }
        
        pos += char_len;
    }
    
    return pos == byte_length;
}

int utf8_char_at_index(const char* str, int char_index, int byte_length) {
    if (str == NULL || char_index < 0 || byte_length < 0) {
        return -1;
    }
    if (char_index == 0) {
        return 0;
    }
    if (byte_length == 0) {
        return -1;
    }
    
    int char_count = 0;
    int pos = 0;
    
    while (pos < byte_length && char_count < char_index) {
        int char_len = utf8_char_length((unsigned char)str[pos]);
        if (char_len <= 0) {
            char_count++;
            pos++;
            continue;
        }
        if (pos + char_len > byte_length) {
            char_count++;
            break;
        }
        
        char_count++;
        pos += char_len;
    }
    
    return (char_count == char_index) ? pos : -1;
}

int utf8_next_char(const char* str, int current_byte_pos, int byte_length) {
    if (str == NULL || current_byte_pos < 0 || byte_length < 0) {
        return -1;
    }
    if (current_byte_pos >= byte_length) {
        return -1;
    }
    
    int char_len = utf8_char_length((unsigned char)str[current_byte_pos]);
    if (char_len <= 0) {
        return (current_byte_pos + 1 < byte_length) ? current_byte_pos + 1 : -1;
    }
    
    int next_pos = current_byte_pos + char_len;
    if (next_pos > byte_length) {
        return -1;
    }
    
    return (next_pos < byte_length) ? next_pos : -1;
}

bool utf8_is_ascii_only(const char* str, int byte_length) {
    if (str == NULL || byte_length < 0) {
        return false;
    }
    if (byte_length == 0) {
        return true;
    }
    
    for (int i = 0; i < byte_length; i++) {
        if ((unsigned char)str[i] > 0x7F) {
            return false;
        }
    }
    
    return true;
}

int utf8_string_length_fast(const char* str, int byte_length) {
    if (str == NULL || byte_length < 0) {
        return 0;
    }
    if (byte_length == 0) {
        return 0;
    }
    
    bool is_ascii = true;
    for (int i = 0; i < byte_length; i++) {
        if ((unsigned char)str[i] > 0x7F) {
            is_ascii = false;
            break;
        }
    }
    
    if (is_ascii) {
        return byte_length;
    }
    
    return utf8_string_length(str, byte_length);
}

int utf8_get_cached_length(struct ObjString* string) {
    if (string == NULL || string->chars == NULL) {
        return 0;
    }
    
    if (string->char_length != -1) {
        return string->char_length;
    }
    
    bool is_ascii = utf8_is_ascii_only(string->chars, string->length);
    string->is_ascii = is_ascii;
    
    if (is_ascii) {
        string->char_length = string->length;
    } else {        
        string->char_length = utf8_string_length(string->chars, string->length);
    }
    
    return string->char_length;
}

bool utf8_get_cached_is_ascii(struct ObjString* string) {
    if (string == NULL || string->chars == NULL) {
        return true;
    }
    
    if (string->char_length != -1) {
        return string->is_ascii;
    }
    
    utf8_get_cached_length(string);
    return string->is_ascii;
}
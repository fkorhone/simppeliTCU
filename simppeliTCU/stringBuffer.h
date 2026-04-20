#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// Fixed size string buffer to avoid heap fragmentation
template <size_t N>
class StringBuffer {
private:
    char buffer[N];
public:
    StringBuffer() { buffer[0] = '\0'; }
    
    void clear() { buffer[0] = '\0'; }
    
    bool isEmpty() const { return buffer[0] == '\0'; }
    
    const char* c_str() const { return buffer; }
    
    size_t length() const { return strlen(buffer); }
    
    void copyFrom(const char* str) {
        strncpy(buffer, str, N);
        buffer[N - 1] = '\0';
    }
    
    void copyFromData(const byte* data, size_t len) {
        size_t copylen = (len < N - 1) ? len : (N - 1);
        memcpy(buffer, data, copylen);
        buffer[copylen] = '\0';
    }
    
    void format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, N, fmt, args);
        va_end(args);
    }
    
    void append(const char* str) {
        strncat(buffer, str, N - length() - 1);
    }
    
    bool startsWith(const char* prefix) const {
        return strncmp(buffer, prefix, strlen(prefix)) == 0;
    }
    
    bool equals(const char* str) const {
        return strcmp(buffer, str) == 0;
    }
    
    void trim() {
        // Trim right
        for (int i = (int)length() - 1; i >= 0; i--) {
            if (isspace((unsigned char)buffer[i])) {
                buffer[i] = '\0';
            } else {
                break;
            }
        }
        // Trim left
        char* start = buffer;
        while (isspace((unsigned char)*start)) {
            start++;
        }
        if (start != buffer) {
            memmove(buffer, start, strlen(start) + 1);
        }
    }

    void toLowerCase() {
        for (int i = 0; buffer[i]; i++) {
            buffer[i] = (char)tolower((unsigned char)buffer[i]);
        }
    }
};

#endif // STRING_BUFFER_H

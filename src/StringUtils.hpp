#pragma once

// Does not require NULL termination
int custom_atoi(char* str);

#ifndef PC
#include <stdio.h>
#endif

// Returns false could not find (Also seeks back to original location)
// Returns true if could find target
#ifndef PC
bool seek_next_char(FILE* fd, char target);
#else
bool seek_next_char(int fd, char target);
#endif

// Reads string into buff from the current position
// till the target character. The target character is
// also included. File position is then at the next
// character after target. It WILL modify the buff
// even if it was not successful.
// Returns: true when successful
//          false when not succesful (also seeks back to original location)
#ifndef PC
bool read_until(FILE* fd, char* buf, int buf_size, char target, bool include_target);
#else
bool read_until(int fd, char* buf, int buf_size, char target, bool include_target);
#endif

// Assumes Linefeed '\n' (LF) as the end of line character
// Use "dos2unix" on your file to turn them into LF when using files from windows.
// Windows default is Carriage + Linefeed \r\n (CRLF) which is 2 characters.
// TODO: Add another read_until that has target
//       string instead of single character.
#ifndef PC
bool read_line(FILE* fd, char* buf, int buf_size);
#else
bool read_line(int fd, char* buf, int buf_size);
#endif

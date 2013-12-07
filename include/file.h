#ifndef FILE_H_
#define FILE_H_

#include <stdbool.h>
#include <unistd.h>

char* read_file(FILE* file);
char* md5sum(FILE* file);
bool file_changed(const char* path);

#endif  // FILE_H_

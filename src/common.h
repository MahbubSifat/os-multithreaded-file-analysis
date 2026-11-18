#ifndef COMMON_H
#define COMMON_H
#include <stddef.h>

typedef struct {
    char **paths;
    size_t count;
} FileList;

int  list_txt_files(const char *dir, FileList *out);
void free_file_list(FileList *fl);
long count_word_in_file(const char *path, const char *word);
double now_sec(void);

#endif

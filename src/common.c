#define _GNU_SOURCE
#include "common.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* qsort er jonno: duita file er nam tulona kore */
static int cmp_paths(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

/* Folder er shob .txt file er talika banay */
int list_txt_files(const char *dir, FileList *out) {
    DIR *d = opendir(dir);
    if (!d) { perror(dir); return -1; }

    size_t cap = 64;
    out->count = 0;
    out->paths = malloc(cap * sizeof(char *));
    if (!out->paths) { closedir(d); return -1; }

    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        size_t n = strlen(e->d_name);
        if (n < 5 || strcmp(e->d_name + n - 4, ".txt") != 0) continue;

        if (out->count == cap) {
            cap *= 2;
            char **tmp = realloc(out->paths, cap * sizeof(char *));
            if (!tmp) { free_file_list(out); closedir(d); return -1; }
            out->paths = tmp;
        }
        size_t len = strlen(dir) + n + 2;
        char *p = malloc(len);
        if (!p) { free_file_list(out); closedir(d); return -1; }
        snprintf(p, len, "%s/%s", dir, e->d_name);
        out->paths[out->count++] = p;
    }
    closedir(d);
    qsort(out->paths, out->count, sizeof(char *), cmp_paths);
    return 0;
}

/* Talika er memory free kore */
void free_file_list(FileList *fl) {
    for (size_t i = 0; i < fl->count; i++) free(fl->paths[i]);
    free(fl->paths);
    fl->paths = NULL;
    fl->count = 0;
}

/* Ekta file e ekta word koybar ache gone */
long count_word_in_file(const char *path, const char *word) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return -1; }

    char *line = NULL;
    size_t cap = 0;
    long count = 0;

    while (getline(&line, &cap, f) != -1) {
        char *save = NULL;
        for (char *tok = strtok_r(line, " \t\r\n", &save); tok;
             tok = strtok_r(NULL, " \t\r\n", &save)) {
            if (strcmp(tok, word) == 0) count++;
        }
    }
    free(line);
    fclose(f);
    return count;
}

/* Ekhon er somoy second e (somoy mapar jonno) */
double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

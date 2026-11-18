/* INTENTIONALLY WRONG: race condition demo (kono lock nai) */
#define _GNU_SOURCE
#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Shob thread ei EKTAI shared sonkhya e jog kore, lock chhara */
volatile long shared_total = 0;

typedef struct {
    int id;
    int nthreads;
    const FileList *fl;
    const char *word;
} ThreadArg;

static void count_racy(const char *path, const char *word) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return; }
    char *line = NULL;
    size_t cap = 0;
    while (getline(&line, &cap, f) != -1) {
        char *save = NULL;
        for (char *tok = strtok_r(line, " \t\r\n", &save); tok;
             tok = strtok_r(NULL, " \t\r\n", &save)) {
            if (strcmp(tok, word) == 0) shared_total++;   /* <-- RACE HERE */
        }
    }
    free(line);
    fclose(f);
}

static void *worker(void *p) {
    ThreadArg *a = p;
    for (size_t i = (size_t)a->id; i < a->fl->count; i += (size_t)a->nthreads)
        count_racy(a->fl->paths[i], a->word);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <directory> <word> <threads>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[3]);
    if (n < 1 || n > 256) {
        fprintf(stderr, "threads must be between 1 and 256\n");
        return 1;
    }

    FileList fl;
    if (list_txt_files(argv[1], &fl) != 0) return 1;

    pthread_t tids[256];
    ThreadArg args[256];
    int created = 0;
    for (int i = 0; i < n; i++) {
        args[i] = (ThreadArg){ i, n, &fl, argv[2] };
        if (pthread_create(&tids[i], NULL, worker, &args[i]) != 0) break;
        created++;
    }
    for (int i = 0; i < created; i++) pthread_join(tids[i], NULL);

    printf("RACY: files=%zu word=%s threads=%d total=%ld\n",
           fl.count, argv[2], n, (long)shared_total);
    free_file_list(&fl);
    return 0;
}

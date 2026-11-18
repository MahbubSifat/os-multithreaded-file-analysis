/* FIXED with mutex: prottek match e lock nei, jog kori, unlock kori */
#define _GNU_SOURCE
#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Shared sonkhya + tar talar chabi */
static long shared_total = 0;
static pthread_mutex_t total_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int id;
    int nthreads;
    const FileList *fl;
    const char *word;
} ThreadArg;

static void count_locked(const char *path, const char *word) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return; }
    char *line = NULL;
    size_t cap = 0;
    while (getline(&line, &cap, f) != -1) {
        char *save = NULL;
        for (char *tok = strtok_r(line, " \t\r\n", &save); tok;
             tok = strtok_r(NULL, " \t\r\n", &save)) {
            if (strcmp(tok, word) == 0) {
                pthread_mutex_lock(&total_lock);     /* tala nao */
                shared_total++;                      /* ekjon i likhbe */
                pthread_mutex_unlock(&total_lock);   /* tala chharo */
            }
        }
    }
    free(line);
    fclose(f);
}

static void *worker(void *p) {
    ThreadArg *a = p;
    for (size_t i = (size_t)a->id; i < a->fl->count; i += (size_t)a->nthreads)
        count_locked(a->fl->paths[i], a->word);
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
    if (fl.count == 0) {
        fprintf(stderr, "No .txt files found in %s\n", argv[1]);
        free_file_list(&fl);
        return 1;
    }

    pthread_t tids[256];
    ThreadArg args[256];
    int created = 0;

    double t0 = now_sec();
    for (int i = 0; i < n; i++) {
        args[i] = (ThreadArg){ i, n, &fl, argv[2] };
        if (pthread_create(&tids[i], NULL, worker, &args[i]) != 0) {
            fprintf(stderr, "pthread_create failed for thread %d\n", i);
            break;
        }
        created++;
    }
    for (int i = 0; i < created; i++) pthread_join(tids[i], NULL);
    double t1 = now_sec();

    if (created != n) {
        fprintf(stderr, "ERROR: only %d of %d threads started, result incomplete\n",
                created, n);
        free_file_list(&fl);
        return 1;
    }

    printf("MUTEX: files=%zu word=%s threads=%d total=%ld time=%.4f\n",
           fl.count, argv[2], n, shared_total, t1 - t0);
    free_file_list(&fl);
    return 0;
}

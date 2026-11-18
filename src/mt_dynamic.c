/* Dynamic work queue: je thread free, se porer file ta tule ney */
#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Shared kajer line: porer file er nombor + tar tala */
static size_t next_index = 0;
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int id;
    const FileList *fl;
    const char *word;
    long local_count;     /* ONLY ei thread er nijer count */
    int skipped;
    int files_done;       /* ei thread koyta file korlo */
    double busy;          /* ei thread koto second kaj korlo */
} ThreadArg;

static void *worker(void *p) {
    ThreadArg *a = p;
    double t0 = now_sec();

    for (;;) {
        /* porer file er nombor nao (tala diye, 1 bar prottek file e) */
        pthread_mutex_lock(&queue_lock);
        size_t i = next_index++;
        pthread_mutex_unlock(&queue_lock);

        if (i >= a->fl->count) break;          /* file shesh */

        long c = count_word_in_file(a->fl->paths[i], a->word);
        if (c < 0) { a->skipped++; continue; }
        a->local_count += c;
        a->files_done++;
    }
    a->busy = now_sec() - t0;
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: %s <directory> <word> <threads> [-v]\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[3]);
    int verbose = (argc == 5 && strcmp(argv[4], "-v") == 0);
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

    pthread_t *tids = calloc((size_t)n, sizeof(pthread_t));
    ThreadArg *args = calloc((size_t)n, sizeof(ThreadArg));
    if (!tids || !args) {
        fprintf(stderr, "out of memory\n");
        free(tids); free(args); free_file_list(&fl);
        return 1;
    }

    double t0 = now_sec();
    int created = 0;
    for (int i = 0; i < n; i++) {
        args[i].id = i;
        args[i].fl = &fl;
        args[i].word = argv[2];
        if (pthread_create(&tids[i], NULL, worker, &args[i]) != 0) {
            fprintf(stderr, "pthread_create failed for thread %d\n", i);
            break;
        }
        created++;
    }

    long total = 0;
    int skipped = 0;
    for (int i = 0; i < created; i++) {
        pthread_join(tids[i], NULL);
        total += args[i].local_count;
        skipped += args[i].skipped;
    }
    double t1 = now_sec();

    if (created != n) {
        fprintf(stderr, "ERROR: only %d of %d threads started, result incomplete\n",
                created, n);
        free(tids); free(args); free_file_list(&fl);
        return 1;
    }
    if (skipped > 0) fprintf(stderr, "warning: %d file(s) skipped\n", skipped);

    printf("files=%zu word=%s threads=%d total=%ld time=%.4f\n",
           fl.count, argv[2], n, total, t1 - t0);

    if (verbose)
        for (int i = 0; i < n; i++)
            printf("  thread %d: files=%d busy=%.4f\n",
                   i, args[i].files_done, args[i].busy);

    free(tids); free(args); free_file_list(&fl);
    return 0;
}

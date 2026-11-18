/* Multithreaded version: thread-local result (race condition nai) */
#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* Prottek thread ke ei "kajer chithi" deya hobe */
typedef struct {
    int id;                 /* thread er number: 0,1,2... */
    int nthreads;           /* mot koyta thread */
    const FileList *fl;     /* file talika (shobai pore, kew likhe na) */
    const char *word;       /* je word khujbo */
    long local_count;       /* ONLY ei thread er nijer count */
    int skipped;            /* koyta file pora jayni */
} ThreadArg;

/* Thread er kaj: nijer vager file gulo gone */
static void *worker(void *p) {
    ThreadArg *a = p;
    long sum = 0;
    int skipped = 0;

    for (size_t i = (size_t)a->id; i < a->fl->count; i += (size_t)a->nthreads) {
        long c = count_word_in_file(a->fl->paths[i], a->word);
        if (c < 0) { skipped++; continue; }
        sum += c;
    }
    a->local_count = sum;
    a->skipped = skipped;
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

    pthread_t *tids = calloc((size_t)n, sizeof(pthread_t));
    ThreadArg *args = calloc((size_t)n, sizeof(ThreadArg));
    if (!tids || !args) {
        fprintf(stderr, "out of memory\n");
        free(tids); free(args); free_file_list(&fl);
        return 1;
    }

    double t0 = now_sec();

    /* 1) thread gulo chalu kori */
    int created = 0;
    for (int i = 0; i < n; i++) {
        args[i].id = i;
        args[i].nthreads = n;
        args[i].fl = &fl;
        args[i].word = argv[2];
        if (pthread_create(&tids[i], NULL, worker, &args[i]) != 0) {
            fprintf(stderr, "pthread_create failed for thread %d\n", i);
            break;
        }
        created++;
    }

    /* 2) shobar jonno opekkha kori (jotogulo chalu hoyechhe) */
    long total = 0;
    int skipped = 0;
    for (int i = 0; i < created; i++) {
        pthread_join(tids[i], NULL);
        total += args[i].local_count;      /* 3) sobar count jog */
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

    free(tids); free(args); free_file_list(&fl);
    return 0;
}

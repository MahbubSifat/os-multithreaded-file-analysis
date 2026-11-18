/* Sequential baseline: ekta folder er shob .txt file e ekta word gone */
#include "common.h"
#include <stdio.h>

int main(int argc, char **argv) {
    /* Ke thik moto command dilo kina check */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <word>\n", argv[0]);
        return 1;
    }

    /* Folder er file talika banai */
    FileList fl;
    if (list_txt_files(argv[1], &fl) != 0) return 1;
    if (fl.count == 0) {
        fprintf(stderr, "No .txt files found in %s\n", argv[1]);
        free_file_list(&fl);
        return 1;
    }

    /* Ghori chalu, ek ek file er count jog kori, ghori bondho */
    double t0 = now_sec();
    long total = 0;
    for (size_t i = 0; i < fl.count; i++) {
        long c = count_word_in_file(fl.paths[i], argv[2]);
        if (c < 0) {
            fprintf(stderr, "skipped: %s\n", fl.paths[i]);
            continue;
        }
        total += c;
    }
    double t1 = now_sec();

    printf("files=%zu word=%s total=%ld time=%.4f\n",
           fl.count, argv[2], total, t1 - t0);

    free_file_list(&fl);
    return 0;
}

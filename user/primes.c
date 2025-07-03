#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(2, "primes: usage: primes\n");
        exit(1);
    }

    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "primes: pipe failed\n");
        exit(1);
    }

    if (fork() == 0) {
        // Child process
        close(p[1]); // Close write end
        int prime;
        while (read(p[0], &prime, sizeof(prime)) > 0) {
            fprintf(1, "prime %d\n", prime);
        }
        close(p[0]);
        exit(0);
    } else {
        // Parent process
        close(p[0]); // Close read end
        for (int i = 2; i < 280; i++) {
            int is_prime = 1;
            for (int j = 2; j * j <= i; j++) {
                if (i % j == 0) {
                    is_prime = 0;
                    break;
                }
            }
            if (is_prime) {
                write(p[1], &i, sizeof(i));
            }
        }
        close(p[1]); // Close write end
        wait(0); // Wait for child to finish
    }

    exit(0);
}
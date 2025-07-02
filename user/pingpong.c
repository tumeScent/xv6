#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]){

    if(argc != 1){
        fprintf(2, "pingpong: usage: pingpong\n");
        exit(1);
    }

    int p[2];
    char buf[10];
    if(pipe(p) < 0){
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }
    if(fork() == 0){
        //child process
        while(read(p[0], buf, sizeof(buf)) <= 0){
            // wait for the parent to write
        };
        fprintf(1, "%d:recieved ping\n", getpid());
        if(write(p[1], buf, sizeof(buf)) < 0){
            fprintf(2, "pingpong: write failed\n");
            exit(1);   
        }
    }
    else{
        //father process
        strcpy(buf, "SECRET");
        if(write(p[1], buf, sizeof(buf)) < 0){
            fprintf(2, "pingpong: write failed\n");
            exit(1);
        }
        wait(0); // wait for child to finish
        read(p[0], buf, sizeof(buf));
        fprintf(1, "%d:received pong\n", getpid());
        if(strcmp(buf, "SECRET") != 0){
            fprintf(2, "pingpong: received wrong message\n");
            exit(1);
        }
    }

    exit(0);

}
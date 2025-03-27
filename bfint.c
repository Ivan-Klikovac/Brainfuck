#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int print_mem = 20; // the limit above which memory won't be printed after program execution
int MMAX = 65536; // maximum memory for the interpreted program

// given a ptr to [ or ], find the matching parenthesis
char* match_loop(char* code, char* insp)
{
    if(!(*insp == '[' || *insp == ']')) goto bad_args;

    if(*insp == '[') {
        // loop forward until you find n+1 closed parentheses,
        // where n is the number of open parentheses found along the way
        char* ptr = insp+1; // start from the next character
        int n = 0;
        while(*ptr) {
            if(*ptr == '[') n++;
            if(*ptr == ']') n--;
            if(n == -1) return ptr;
            ptr++;
        }
        // didn't find the matching parenthesis
        printf("Unmatched open loop instruction '[' at insp %ld\n", insp - code);
    }

    if(*insp == ']') {
        // loop backward until you find n+1 open parentheses,
        // where n is the number of closed parentheses found along the way
        char* ptr = insp-1;
        int n = 0;
        while(ptr > code) {
            if(*ptr == '[') n--;
            if(*ptr == ']') n++;
            if(n == -1) return ptr;
            ptr--;
        }

        printf("Unmatched close loop instruction ']' at insp %ld\n", insp - code);
    }

    printf("match_loop: failed to find the matching loop instruction\n");
    exit(0);

    bad_args:
    printf("match_loop: not a parenthesis\n");
    exit(0);
}

// returns the number of bytes that are nonzero,
// and sets start and end to point to where nonzero memory begins and ends, respectively
size_t used_memory(char* mem, char** start, char** end)
{
    char* unused_l = mem;
    while(!(*unused_l) && unused_l < mem + MMAX - 1) unused_l++;

    char* unused_r = mem + MMAX - 1;
    while(!(*unused_r) && unused_r > mem) unused_r--;

    *start = unused_l;
    *end = unused_r;

    return unused_r - unused_l;
}

void print_memory(char* start, char* end)
{
    printf("--- Program execution over, final memory state:\n--- [ ");

    while(start <= end) {
        printf("%X ", *start);
        start++;
    }
    printf("]\n\n");
}

void interpret(char* code)
{
    printf("--- PROGRAM OUTPUT:\n");

    char* mem = calloc(MMAX, 1);
    if(!mem) goto no_mem;
    
    char* insp = code; // instruction pointer; it'll move through the passed code array
    int data = 0; // data pointer; it'll move through the allocated array

    while(*insp) {
        if(*insp == '+') {
            mem[data]++;
            goto next;
        }

        if(*insp == '-') {
            mem[data]--;
            goto next;
        }

        if(*insp == '>') {
            data++;
            goto next;
        }

        if(*insp == '<') {
            data--;
            goto next;
        }

        if(*insp == ',') {
            mem[data] = getchar();
            goto next;
        }

        if(*insp == '.') {
            putchar(mem[data]);
            goto next;
        }

        if(*insp == '[') {
            if(mem[data] == 0) insp = match_loop(code, insp);
            goto next; // goto next will move insp by 1 more, to the instruction after the loop
        }

        if(*insp == ']') {
            insp = match_loop(code, insp);
            continue; // has to stay on the conditional jump instruction to evaluate the condition
        }

        else {
            printf("Unexpected token: '%c'\n", *insp);
            exit(0);
        }

        next:
        if(data > MMAX) {
            // idk what the standard specifies in this case
            printf("Data pointer overflow at insp %ld\n", insp - code);
            exit(0);
        }

        insp++;
    }

    // check if the final memory state should be printed
    char* start;
    char* end;
    size_t used_mem = used_memory(mem, &start, &end);
    if(used_mem > print_mem) goto end;
    print_memory(start - 10 < mem ? mem : start - 10,
    end + 10 > mem + MMAX - 1 ? mem + MMAX - 1 : end + 10);

    goto end;
    
    no_mem:
    printf("Failed to allocate %d bytes\n", MMAX);
    goto end;

    end:
    return;
}

int main(int argc, char* argv[])
{
    // add argument parsing for things like memory size, printing the final memory state, etc
    if(argc < 2) goto no_args;
    FILE* file = fopen(argv[1], "rb");
    if(!file) goto bad_file;
    
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    rewind(file);

    char* code = malloc(fsize + 1);
    if(!code) goto no_mem;
    size_t read = fread(code, 1, fsize, file);
    if(read != fsize) goto bad_read;
    fclose(file);

    //code = preprocess(code);
    interpret(code);

    goto end;

    no_args:
    printf("Usage: bfint [file]\nRun bfint --help for more info");
    goto end;

    bad_file:
    printf("Failed to open %s\n", argv[1]);
    goto end;

    no_mem:
    printf("Failed to allocate %ld bytes\n", fsize+1);
    goto end;

    bad_read:
    printf("Failed to read from file (read %ld/%ld bytes)\n", read, fsize);
    goto end;

    end:
    free(code);
    return 0;
}
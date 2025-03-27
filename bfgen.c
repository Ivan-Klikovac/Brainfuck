#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define minstr2(X, Y) ((strlen(X)) < (strlen(Y)) ? (X) : (Y))

char decval = '-';
char incval = '+';
char decptr = '<';
char incptr = '>';
char loopstart = '[';
char loopend = ']';
char input = ',';
char output = '.';

// inferior algorithm:
// first sets all character cells to the smallest ASCII value
// I don't see a case where this algorithm will outperform avg
char* generate_min(char* input)
{
    char* text = strdup(input);
    char* code = malloc(strlen(text) * 150);
    memset(code, 0, strlen(text) * 150);
    int insp = 0;

    char min = CHAR_MAX;
    for(int i = 0; text[i]; i++) if(text[i] < min) min = text[i];
    
    code[insp++] = incptr;
    for(int i = 0; i < min; i++) code[insp++] = incval;

    code[insp++] = loopstart;
    for(int i = 0; i < strlen(text); i++) {
        code[insp++] = incptr;
        code[insp++] = incval;
    }

    // inner loop to go back to the min counter cell
    code[insp++] = loopstart;
    code[insp++] = decptr;
    code[insp++] = loopend;
    
    // decrement the counter and loop
    code[insp++] = incptr;
    code[insp++] = decval;
    code[insp++] = loopend;

    code[insp++] = incptr;

    for(int i = 0; i < strlen(text); i++) {
        while(text[i] > min) {
            code[insp++] = incval;
            text[i]--;
        }
        code[insp++] = incptr;
    }

    code[insp++] = decptr;
    code[insp++] = loopstart;
    code[insp++] = decptr;
    code[insp++] = loopend;
    
    code[insp++] = incptr; 
    code[insp++] = loopstart;
    code[insp++] = output;
    code[insp++] = incptr;
    code[insp++] = loopend;

    code[insp] = 0;

    printf("min: %ld -> %ld (+%.2lf%%)\n", strlen(text), strlen(code), (double)strlen(code)/strlen(text) * 100);
    free(text);
    return code;
}

// superior algorithm:
// first sets all character cells to the average ASCII value
char* generate_avg(char* input)
{
    // returns Brainfuck code that prints the given text
    // how do I estimate the amount of memory it'll take up?
    // no idea. let's just allocate a reasonably large amount
    char* text = strdup(input);
    char* code = malloc(strlen(text) * 100);
    memset(code, 0, strlen(text) * 100);
    int insp = 0; // instruction pointer

    // I'll first want to find the "average" ASCII value of all characters in the text
    long long avg = 0; // this won't quite be enough for large enough inputs
    // maybe solve it with floats later? add a character divided by strlen in each iteration
    // could make it unsigned to squeeze out an extra bit
    // but this will require some rethinking... later.
    for(int i = 0; i < strlen(text); i++) avg += text[i];
    avg /= strlen(text);
    if(avg < 0) goto wtf;

    // now that I have that, uh...
    // set the first cell to contain the average
    code[insp++] = incptr; // leave the zero cell with value 0
    // OKAYYY instead of setting a cell value to n by putting n incvals, let's...
    // let's be a bit smarter about this. a cell can be set to the number 25 by setting a loop counter to 5
    // and having a loop that increments the cell by 5 in each iteration
    // which brings it down from 25 characters to 15 or so, which would scale "inversely" with the desired number
    // I'll do that later, I seriously cba to remember how tf this code works rn
    for(int i = 0; i < avg; i++) code[insp++] = incval;
    // now make a loop that sets all other cells to that average
    code[insp++] = loopstart;
    // now before putting the matching ] to end the loop,
    // I'll need to go through every cell that's supposed to contain a character
    // there will be strlen(text) of them
    // then I return to the zero cell, decrement it, and close the loop
    for(int i = 0; i < strlen(text); i++) {
        code[insp++] = incptr;
        code[insp++] = incval;
    }
    code[insp++] = loopstart;
    code[insp++] = decptr;
    code[insp++] = loopend;
    
    code[insp++] = incptr; // go to the average counter
    code[insp++] = decval; // decrement it
    code[insp++] = loopend;

    // all cells after the zero cell are now set to the average ASCII character
    // now I'll have to increment/decrement each of them to get the desired character
    // the current selected cell is the zero cell
    code[insp++] = incptr; // go to the first character cell

    // for instance, if I have character 'a' and average 'd'
    // since all cells are set to the average,
    // I want to decrement the cell by 3
    // while incrementing the string so the code can keep track of it
    for(int i = 0; i < strlen(text); i++) {
        while(text[i] < avg) {
            code[insp++] = decval;
            text[i]++;
        }
        while(text[i] > avg) {
            code[insp++] = incval;
            text[i]--;
        }
        code[insp++] = incptr;
    }
    
    // all ASCII codes are set now
    // go back to the first character
    code[insp++] = decptr; // first go back one character, to not be on a zero
    code[insp++] = loopstart; // then loop to the beginning
    code[insp++] = decptr;
    code[insp++] = loopend;
    
    // and start printing
    code[insp++] = incptr; // move from the zero cell
    code[insp++] = loopstart; // then loop forward and print
    code[insp++] = output;
    code[insp++] = incptr;
    code[insp++] = loopend;

    code[insp] = 0;

    printf("average: %ld -> %ld (+%.2lf%%)\n", strlen(text), strlen(code), (double)strlen(code)/strlen(text) * 100);
    free(text);
    return code;

    wtf:
    printf("huh?\n");
    free(text);
    exit(1);
}

int main(int argc, char* argv[])
{
    if(argc < 2) goto no_args;
    if(strncmp(argv[1], "--help", 6) == 0) goto help;
    if(argv[2] && strncmp(argv[2], "--cmds=", 7) == 0) goto given_cmdset;
    if(argv[3] && strncmp(argv[3], "--cmds=", 7) == 0) goto given_cmdset;
    if(strncmp(argv[1], "--input=", 8) == 0) goto given_input;

    goto no_args;

    given_input:;
    FILE* file = fopen(argv[1] + 8, "rb");
    if(!file) goto bad_input_file;
    
    // get file size
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    rewind(file);

    char* text = malloc(fsize + 1);
    if(!text) goto no_mem;
    size_t read = fread(text, 1, fsize, file);
    if(read != fsize) goto bad_input_read;
    fclose(file);
    text[fsize] = 0;
    
    char* result_avg = generate_avg(text);
    //char* result_min = generate_min(text);
    char* result = result_avg; //minstr2(result_avg, result_min);

    if(argv[2] && strncmp(argv[2], "--output=", 9) == 0) {
        FILE* outfile = fopen(argv[2] + 9, "wb");
        if(!outfile) goto bad_output_file;
        size_t write = fwrite(result, 1, strlen(result) + 1, outfile);
        if(write != strlen(result) + 1) goto bad_output_write;
    }

    else {
        // print to stdout
        printf("%s", result);
    }

    return 0;

    given_cmdset:;
    int cmdset_index = (argv[2] && strncmp(argv[2], "--cmds=", 7) == 0) ? 2 : 3;
    decval = argv[cmdset_index][7];
    incval = argv[cmdset_index][8];
    decptr = argv[cmdset_index][9];
    incptr = argv[cmdset_index][10];
    loopstart = argv[cmdset_index][11];
    loopend = argv[cmdset_index][12];
    input = argv[cmdset_index][13];
    output = argv[cmdset_index][14];
    if(strncmp(argv[1], "--input=", 8) == 0) goto given_input;
    else goto no_args;

    no_args:
    printf("Usage: bfgen --input=[source] --output=[output]\nRun bfgen --help for more info\n");
    return 1;

    help:
    printf("--- Brainfuck code generator v0.1 [CRADLE]\n\n");
    printf("Usage: bfgen --input=[source] --output=[output] --cmds=[cmdset]\n\n");
    printf("--output is optional; if no output is specified, prints to stdin\n");
    printf("--cmds is optional; if no cmdset is specified, ");
    printf("uses the standard character set for Brainfuck commands.\n\n");
    printf("cmdset is an 8-character string where each character corresponds ");
    printf("to a Brainfuck command; in this order: -+<>[],.\n");
    printf("You can specify a file in --cmds, in which case the code generator ");
    printf("reads the first eight characters of the file and uses them as cmdset.\n");
    printf("You typically want to encase the --cmds argument in quotes, such as ");
    printf("'--cmds=-+<>[],.' in order to ensure the argument is passed properly.\n");
    printf("Support for a set of strings as the command set will be added at some point.\n");
    return 0;   

    no_mem:
    printf("Failed to allocate %ldB\n", fsize + 1);
    return 1;

    bad_input_file:
    printf("Failed to open %s\n", argv[1] + 8);
    return 1;

    bad_input_read:
    printf("Couldn't read %s properly (%ld/%ldB)\n", argv[1] + 8, fsize, read);
    return 1;

    bad_output_file:
    printf("Failed to open %s\n", argv[2] + 9);
    return 1;

    bad_output_write:
    printf("Couldn't properly write to %s\n", argv[2] + 9);
    return 1;
}
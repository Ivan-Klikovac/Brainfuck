#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char decval = '-';
char incval = '+';
char decptr = '<';
char incptr = '>';
char loopstart = '[';
char loopend = ']';
char input = ',';
char output = '.';

static inline int is_instruction(char c)
{
    return c == decval || c == incval || c == decptr || c == incptr
    || c == loopstart || c == loopend || c == input || c == output;
}

void remove_character(char* c)
{
    while(*c) {
        *c = *(c+1);
        c++;
    }
    *(c-1) = 0;
}

void preprocess(char* code)
{
    char* temp = code;
    while(*temp) {
        if(!is_instruction(*temp)) remove_character(temp);
        else temp++;
    }

    // after deleting all non-commands, swap out the characters
    while(*code) {
        if(*code == decval) *code = '-';
        if(*code == incval) *code = '+';
        if(*code == decptr) *code = '<';
        if(*code == incptr) *code = '>';
        if(*code == loopstart) *code = '[';
        if(*code == loopend) *code = ']';
        if(*code == input) *code = ',';
        if(*code == output) *code = '.';
        code++;
    }
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

    char* code = malloc(fsize + 1);
    if(!code) goto no_mem;
    size_t read = fread(code, 1, fsize, file);
    if(read != fsize) goto bad_input_read;
    fclose(file);
    code[fsize] = 0;

    preprocess(code);

    if(argv[2] && strncmp(argv[2], "--output=", 9) == 0) {
        FILE* outfile = fopen(argv[2] + 9, "wb");
        if(!outfile) goto bad_output_file;
        size_t write = fwrite(code, 1, strlen(code) + 1, outfile);
        if(write != strlen(code) + 1) goto bad_output_write;
    }

    else {
        // print to stdout
        printf("%s", code);
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
    printf("Usage: bfproc --input=[source] --output=[output]\nRun bfproc --help for more info\n");
    return 1;

    help:
    printf("--- Brainfuck preprocessor v0.1 [CRADLE]\n\n");
    printf("Usage: bfproc --input=[source] --output=[output] --cmds=[cmdset]\n\n");
    printf("--output is optional; if no output is specified, prints to stdin\n");
    printf("--cmds is optional; if no cmdset is specified, ");
    printf("uses the standard character set for Brainfuck commands.\n\n");
    printf("cmdset is an 8-character string where each character corresponds ");
    printf("to a Brainfuck command; in this order: -+<>[],.\n");
    printf("You can specify a file in --cmds, in which case the preprocessor ");
    printf("reads the first eight characters of the file and uses them as cmdset.\n");
    printf("You typically want to encase the --cmds argument in quotes, such as ");
    printf("'--cmds=-+<>[],.' in order to ensure the argument is passed properly.\n");
    printf("Support for a set of strings as the command set will be added at some point.\n");
    return 0;   

    no_mem:
    printf("Failed to allocate %ld bytes\n", fsize + 1);
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
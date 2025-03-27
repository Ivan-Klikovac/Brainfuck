#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __linux
#include <sys/mman.h>
#include <errno.h>

void* alloc(void)
{
    void* ptr = mmap(0, 1 << 30, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if(ptr == MAP_FAILED) {
        puts(strerror(errno));
        exit(1);
    }

    return ptr;
}

#endif // __linux

#define abs(A) (((A) > 0) ? (A) : (-A))
#define max2(A, B) ((A) > (B) ? (A) : (B))
#define max3(A, B, C) (max2(A, B) > (C) ? max2(A, B) : (C))

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

void assert(int expr, int line, char* comment)
{
    if(!expr) {
        printf("(%s) Failed assertion at line %d\n%s\n", __FILE__, line, comment);
    }
}

// count the number of occurences of c in s
int count(char* s, char c)
{
    int res = 0;

    while(*s) {
        if(*s == c) res++;
        s++;
    }

    return res;
}

// checks if c exists in s; I could collapse this into count()
int exists(char* s, char c)
{
    int res = 0;
    
    while(*s) {
        if(*s == c) {
            res = 1;
            break;
        }
        s++;
    }

    return res;
}

// adds the character c at the place s, pushing the rest of the string to the right
// assumes the string has enough room after the first 0 to add a character
void cadd(char* s, char c)
{
    //if(!s) goto no_string;

    char* start = s;
    char* end = strchr(s, 0);
    s = end;
    *(s+1) = 0; // the previous null term will be trampled
    
    while(s > start) {
        *s = *(s-1);
        s--;
    }

    *s = c;

    //no_string:
    //*s++; // intentionally trigger a segfault
    //printf("cadd: s is NULL\n");
}

// returns a pointer to the first group (3+) of characters c in string s
char* find_group(char* s, char c)
{
    start:
    if(!s) goto no_string;
    char* potential = strchr(s, c);
    if(!potential) return NULL;

    int n = 0;
    while(*potential == c) {
        n++;
        potential++;
    }

    if(n >= 3) return potential - n; // return the ptr to the beginning of the group

    // if the group isn't big enough, move the string pointer forward and repeat
    s = potential + 1;
    goto start;

    no_string:
    printf("find_group: s is NULL\n");
    return NULL;
}

// this initializes the character set to be used for obfuscation
void obfcinit(char* obfc)
{
    char original[8];
    original[0] = decptr;
    original[1] = incptr;
    original[2] = decval;
    original[3] = incval;
    original[4] = loopstart;
    original[5] = loopend;
    original[6] = input;
    original[7] = output;
    int digit = 0;
    int alpha = 0;
    int cntrl = 0;
    
    // taking character groups that resemble the original character set
    // the admittedly ugly breaks ensure the sum never exceeds 8
    for(int i = 0; digit + alpha + cntrl < 8 && i < 8; i++) {
        if(isdigit(original[i])) digit++;
        if(digit + alpha + cntrl == 8) break;
        if(isalpha(original[i])) alpha++;
        if(digit + alpha + cntrl == 8) break;
        if(iscntrl(original[i])) cntrl++;
    }

    // case to be considered: all characters are non-digit, non-alpha and non-control
    if(!digit && !alpha && !cntrl) goto alt;

    if(!digit) goto after_digit;
    char* digitpool = malloc(digit + 1);
    memset(digitpool, 0, digit + 1);
    // set this to random non-instruction digits
    int di = 0; // digit index
    char d;
    rnd_digit:
    if(di == digit || di == 10 - digit) goto after_digit;
    d = rand();
    if(!isdigit(d) || is_instruction(d) || exists(digitpool, d)) goto rnd_digit;
    digitpool[di] = d;
    di++;
    goto rnd_digit;

    after_digit:
    
    if(!alpha) goto after_alpha;
    char* alphapool = malloc(alpha + 1);
    memset(alphapool, 0, alpha + 1);
    int ai = 0;
    char a;
    rnd_alpha:
    if(ai == alpha) goto after_alpha;
    a = rand();
    if(!isalpha(a) || is_instruction(a) || exists(alphapool, a)) goto rnd_alpha;
    alphapool[ai] = a;
    ai++;
    goto rnd_alpha;

    after_alpha:
    
    if(!cntrl) goto after_cntrl;
    char* cntrlpool = malloc(cntrl + 1);
    memset(cntrlpool, 0, cntrl + 1);
    int ci = 0;
    char c;
    rnd_cntrl:
    if(ci == cntrl) goto after_cntrl;
    c = rand(); // !c for null term since I'm not sure if it'll interfere with the programs' functioning
    if(!c || !iscntrl(c) || is_instruction(c) || exists(cntrlpool, c)) goto rnd_cntrl;
    cntrlpool[ci] = c;
    ci++;
    goto rnd_cntrl;

    after_cntrl:;
    // merge these three into a common pool
    char pool[9] = {0};
    // if I don't add these ifs, something strange happens in the case of empty pools
    if(digit) strcat(pool, digitpool);
    if(alpha) strcat(pool, alphapool);
    if(cntrl) strcat(pool, cntrlpool);
    assert(strlen(pool) >= 8, __LINE__, "Character pool has insufficient characters");
    
    for(int i = 0; i < strlen(pool); i++) {
        rnd_pool:;
        int pi = rand() % strlen(pool);
        if(exists(obfc, pool[pi])) goto rnd_pool;
        obfc[i] = pool[pi];
    }

    // the main code will collapse into this if it hasn't gathered 8 characters

    alt:
    while(strlen(obfc) < 8) {
        // just add random characters in here
        rnd:;
        char c = rand();
        if(exists(obfc, c)) goto rnd;
        cadd(obfc, c);
    }
}

char* obfuscate(char* code)
{
    // potential way to go about this:
    
    // decide on eight random, non-command characters and litter the code with them
    // the resulting code will then consist of sixteen different characters

    // glaring problem: parenthesis matching.
    // the code will necessarily have matched pairs of [ and ]
    // this can be obfuscated by randomly adding matched non-command charactrers throughout the code

    #ifndef __linux
    char* result = calloc(1, strlen(code) * 4);
    #else
    char* result = alloc();
    #endif
    strcpy(result, code);

    int insp = 0; // instruction pointer, goes through the original code
    int obfp = 0; // obfuscation pointer, goes through the obfuscated code
    char obfc[9] = {0}; // 8 obfuscation characters
    srand(count(code, incval)); // idk why not
    // maybe it'd be better to hash the command charset for the seed; idk it's rather inconsequential
    obfcinit(obfc);

    // multiple obfuscator passes
    
    // --- FIRST PASS ---
    
    // pepper the code with the first 3 characters from obfc such that they're matched
    // say obfc[0] will be placed symmetrically with obfc[1]
    // do it between n/2 and n times, where n is the number of loops in the original code

    // this covers obfc[0] and obfc[1]
    int nloops = count(code, loopstart);
    if(!nloops) printf("No loops found, have you specified the correct command set?\n");
    int ni = nloops/2 + rand() % (nloops/2); // ni = number of iterations
    for(int i = 0; i < ni; i++) {
        int r = rand() % strlen(code);
        cadd(result + r, obfc[0]);
        cadd(result + strlen(result)-1 - r, obfc[1]);
    }

    int r1;
    int r2;

    for(int i = 0; i < ni; i++) {
        obfc2:
        r1 = rand() % strlen(code);
        r2 = rand() % strlen(code);
        if(r1 == r2) goto obfc2;
        cadd(result + r1, obfc[2]);
        cadd(result + r2, obfc[2]);
    }

    // --- SECOND PASS ---

    // obfc[3] - obfc[7] are free
    // place them throughout the code
    // put groups of say obfc[3] in large groups of incvals
    // put groups of obfc[4] in large groups of decvals
    // put some obfc[5]s in places with incptrs
    // and some obfc[6]s in places with decptrs
    // and then obfc[7]s randomly across the whole code 

    obfc3:;
    char* incvals = find_group(result, incval);
    if(!incvals) goto obfc4;
    if(rand() % 2) cadd(incvals + rand() % 3, obfc[3]); // add obfc[3] at 0, 1, or 2 chars after the start
    goto obfc3; // what happens here is I add one character per "iteration"
    // and repeat until every group of 3+ incvals is broken up by obfuscation characters

    obfc4:;
    char* decvals = find_group(result, decval);
    if(!decvals) goto obfc5;
    if(rand() % 2) cadd(decvals + rand() % 3, obfc[4]);
    goto obfc4;

    obfc5:;
    char* incptrs = find_group(result, incptr);
    if(!incptrs) goto obfc6;
    if(rand() % 2) cadd(incptrs + rand() % 3, obfc[5]);
    goto obfc5;

    obfc6:;
    char* decptrs = find_group(result, decptr);
    if(!decptrs) goto obfc7;
    if(rand() % 2) cadd(decptrs + rand() % 3, obfc[6]);
    goto obfc6;

    obfc7:;
    int i = 0;
    while(*(result + i)) {
        if((rand() % 4) == 0) cadd(result + i, obfc[7]);
        i++;
    }

    printf("The resulting string is %.2fx longer than the original\n", (float) strlen(result) / strlen(code));

    return result;
}

int main(int argc, char* argv[])
{
    // add the option to choose the extent to which it's obfuscated,
    // that is, the amount of garbage to fill it with
    // later

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

    char* result = obfuscate(code);

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
    printf("Usage: bfobf --input=[source] --output=[output]\nRun bfobf --help for more info\n");
    return 1;

    help:
    printf("--- Brainfuck code obfuscator v0.1 [CRADLE]\n\n");
    printf("Usage: bfobf --input=[source] --output=[output] --cmds=[cmdset]\n\n");
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
# Brainfuck
A suite of tools for generating and working with Brainfuck programs.

It includes:
- `bfint`, a Brainfuck interpreter
- `bfgen`, a code generator
- `bfobf`, an obfuscator
- `bfproc`, a preprocessor for deobfuscation

## Installation
Clone the repo and compile whichever file you like. All four C source files are self-contained and independent.

Each program takes a `--help` argument.

## The interpreter - `bfint`

Usage: `bfint` [file]
It assumes standard Brainfuck syntax. The memory limit is set to the minimum required by Brainfuck, 65536 bytes.

`bfint` prints the most relevant part of the memory after the program is done executing.

## The generator - `bfgen`

Usage: `bfgen --input=[source] --output=[output] --cmds=[cmdset]`

It takes any file as input, generates a Brainfuck program that prints the input file, and saves the program into `output`.

The eight characters used to represent Brainfuck commands are specified in `cmdset`. The default is `-+<>[],.`

## The obfuscator - `bfobf`

Usage: `bfobf --input=[source] --output=[output] --cmds=[cmdset]`

It takes a Brainfuck program (with any command set, specified by `cmdset`) as input, litters the code by placing particular characters in misleading ways all over it, and saves the result into `output`.

## The preprocessor - `bfproc`

Usage: analogous to the other two, `bfproc --input=[source] --output=[output] --cmds=[cmdset]`

`bfproc` deobfuscates the given program, and returns it to the standard Brainfuck command set so that it can be directly interpreted afterwards.

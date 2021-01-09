# lasm2

## Goal

- all instructions are defined using a matcher class
- each architecture can implement a matcher. Arch must be chosen at compile time!
- the matcher will determine the size of an instruction
- the matcher is in charge of translating the actual opcodes to binary

Steps:

Scan -> Parse -> Interpret -> output to binary or listing

## Credit:

The parser is largely based on the excellent book [Crafting Interpreters](https://craftinginterpreters.com/) by Bob Nystrom.
Thank you very much for teaching me basic design patterns for interpreters!

# lasm2

## Goal

- all instructions are defined using a matcher class
- each architecture can implement a matcher. Arch must be chosen at compile time!
- the matcher will determine the size of an instruction
- the matcher is in charge of translating the actual opcodes to binary

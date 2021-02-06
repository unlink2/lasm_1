# lasm

lasm is a macro assembler mainly for the 6502 family of CPUs.
It is a direct code to binary assembler. No linking step is currently in place.

## Features

- output binary
- output simple symbol map
- powerful macro system

## Compiling

lasm only depends on the C++ standard library and therefore compiling is straight forward.
To build it you will need:
- A C++ compiler
- libcmocka for unit tests
- git

Run the following commands to build:
```bash
./configure
make
make install
```

## Command line
Currently lasm only has a few command line options.

### Help:
`--help` or `-h`

### Output file
`-output <file>` or `-o <file>`

### Symbol file
`-symbols <file>` or `-s <file`

### General usage
`lasm -s symbols.lst -o binary.bin source.asm`

## Macros and built-in functions

lasm comes with a powerful macro system and built-in functions.

### lo
Returns the lower 8 bits of a 16 bit number.
`lo(0xABCD);` return 0xCD.

### hi
Returns the upper 8 bits of a 16 bit number.
`hi(0xABCD);` returns 0xAB.

### _A
Returns the current address
`_A()`

### org
Sets the current address
`org 0x8000`

### Functions
Functions can be used to compute values and as macros.
Any assembly instruction inside a function will be insterted upon calling it.
```
fn valueFunctuon(x, y) {
    return 100+x+y;
}

fn macroFunction(x) {
    lda #x;
}
```

### Loops
```
for (let i = 0; i < 100; i = i + 1) {
    lda #i;
}

let i = 0;
while (i < 100) {
    i = i + 1;
    lda #i;
}
```

### Define bytes, half words, words and double
```
db "Hello World!";
db 1, 2, 3;
dh 1, 2, 3;
dw 300, 500, 100;
dd 200;
```

### Delarations

```
let i = 100;
```

### Ram segments
```
// bss segment at address 0x100
bss 0x100 {
    ptr 2, // bytes
    flags 1,
}
```

## TODO
- Better error reporting
- Source map output
- Unofficial opcodes
- More architectures?

## Credit:

The parser is largely based on the excellent book [Crafting Interpreters](https://craftinginterpreters.com/) by Bob Nystrom.
Thank you very much for teaching me basic design patterns for interpreters!

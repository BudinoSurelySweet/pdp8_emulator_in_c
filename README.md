# A PDP8 Emulator

This is a PDP8 emulator written in C.

## How to run the project

1. Clone the repo
2. Compile the `src/` directory and include the `include/` directory with a C compiler.
3. Run the executable with `-h` flag to see how to use it.

## Instruction set

```
MRI     I  OPR     ADDRESS
AND     -  000        -
ADD     -  001        - 
LDA     -  010        - 
STA     -  011        - 
BUN     -  100        - 
BSA     -  101        -
ISZ     -  110        -
```

```
RRI     I  OPR     ADDRESS
CLA     0  111  1000 0000 0000
CLE     0  111  0100 0000 0000
CMA     0  111  0010 0000 0000
CME     0  111  0001 0000 0000
CIR     0  111  0000 1000 0000
CIL     0  111  0000 0100 0000
INC     0  111  0000 0010 0000
SPA     0  111  0000 0001 0000
SNA     0  111  0000 0000 1000
SZA     0  111  0000 0000 0100
SZE     0  111  0000 0000 0010
HLT     0  111  0000 0000 0001
```

```
I/O     I  OPR     ADDRESS
INP     1  111  1000 0000 0000
OUT     1  111  0100 0000 0000
```

## Pseudo instructions

```
ORG
DEC
HEX
END
```


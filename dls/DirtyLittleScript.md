# Dirty Little Script (DLS)

i take a look on a different attepmt:  using Flatten Tape

*Note on Highlighting:* DLS does not have `--` for comments only `#` so here I use
**TCSH** highlighter. 

This time it should be really small.

- Lexer: Traditional Tokenizer
- Value: 16 Byte typed with union 
- Parser: Pratt 
- VM: Flatten Tape
- Functionmap from DreiZehn (SymbolTable to function map)

## Features:
- Identifer
    - [X] c-function
    - [X] const
    - [X] var (only globals!)
- [X] Literals int/string
- [X] Math `+` `-` `*` `/` 
- [X] Compare `==` `!=` `<` `>` `<=` `>=`
- [X] C-Funtion Calls 
    - [X] nested with , and ()
    - [X] reverse order POP_STACK)
- [X] If, Elif, else
- [X] Define 
- [X] While 
- Assign `=` 
    - [X] globals

## Time run test_var.dls the first time (2026-10-31)

- using while 
- using math like `a = a + 1` since i dont have inline assign like ++ or *=
- Release with debug

```
 Performance counter stats for './dlsDemo scripts/test_var.dls':

    41,670,890,908      cycles:u                                                              
    80,278,726,253      instructions:u                                                        
    12,138,309,314      branches:u                                                            
        28,661,639      branch-misses:u                                                       
         1,434,520      L1-icache-load-misses:u                                               

      13.562182971 seconds time elapsed

      13.506357000 seconds user
       0.003825000 seconds sys

```

### Same Script with AST-Interpreter DreiZehn

```
 Performance counter stats for './DreiZehn dls/scripts/test_var.dls':

    18,740,336,533      cycles:u                                                              
    52,433,298,431      instructions:u                                                        
    12,751,720,010      branches:u                                                            
         6,225,244      branch-misses:u                                                       
           475,395      L1-icache-load-misses:u                                               

       6.115660131 seconds time elapsed

       6.075584000 seconds user
       0.003937000 seconds sys
```

### Same with ByteCode DreiZehn:

```
 Performance counter stats for './DreiZehn -bc dls/scripts/test_var.dls':

    11,001,946,125      cycles:u                                                              
    33,558,293,791      instructions:u                                                        
     4,289,213,272      branches:u                                                            
        14,701,883      branch-misses:u                                                       
           635,983      L1-icache-load-misses:u                                               

       3.598581406 seconds time elapsed

       3.575701000 seconds user
       0.000997000 seconds sys
       
```
    
So DreiZehn is much faster - i spend a lot of time to optimize it ;) The while 
loop is simular to what i do in DreiZehn bytecode, so I'am  relative sure it's 
the DreiZehn  Variable Pointer Cache and only 8 byte for value instead of 16.
For first run ever 13,50 it's not so bad - still beat python on that ;) 

# DreiZehn Todo ByteCode 

### DECISION pure byte code ot ast/bytecode hybrid 
In OP_CALL_C_FUNCTION i use a pointer from Compile ... this works fine as long
i always compile the AST and not want to save the bytecode. Same for Import 
which is done in Parser. So I can change this all or do a inline bytecode which
depends on the AST Expression. 

- "A": always compile from code - hybrid solution 
- "B": rewrite some parts and make it byte code execution only. 

I guess "B" would by slower. "A": Pushing the C-Function pointer at compiletime 
is faster the look it up every time at runtime. 

    
## Version 0.9 ByteCode

**NOTE:** Ast-Interpreter get slower every line of code i add ^^




- NOTE: IMPORT is not an expression and done directly

    
### NodeTypes:

```
debug::bc 'print "Hello word!"'
global b = 20; debug::bc 'print "Global " b ", " true;'
```

- [X] BlockStatement
- [X] LiteralExpression
- [X] VariableExpression
- [X] ValueExpression
- [X] CallExpression cFunctions only at the moment  ...

---
 
```
> define foo = 47.11; global b = 20; debug::bc 'b = 78;a= 10.1;print "Local:" a "Global " b ", CONST:" true "DEFINE" foo'; print b
0000: OP_CONST Args: 0 
0002: OP_SAVE_VAR Args: 65 
0004: OP_CONST Args: 1 
0006: OP_SET_LOCAL Args: 0 
0008: OP_CONST Args: 2 
0010: OP_GET_LOCAL Args: 0 
0012: OP_CONST Args: 3 
0014: OP_LOAD_VAR Args: 65 
0016: OP_CONST Args: 4 
0018: OP_CONST Args: 5 
0020: OP_CONST Args: 6 
0022: OP_CONST Args: 7 
0024: OP_CALL_C_FUNCTION  Pointer: 0x5617343ab220, Args: 8
Local: 10.100000 Global  78 , CONST: 1 DEFINE 47.110000 
0028: OP_EXIT
78 

```

```
print (debug::bc "math::randomf; math::randomf; math::randomf; math::randomf")
global r = 0;  (debug::bc "r = math::randomf"); print r;
```
- [X] Stack grow also if we have no assign!! mhh where to handle this ...
        ==> need to add a param to compile and add POP_STACK if we have no use
- [X] bytecode must handle object assignment / garbage collection! Global is set by internalSetVariable but 
local is not assigned at all so if a gc is running we are in trouble! 
    `global r = 0; print (debug::bc "r = Array::new; l=Array::new; core::printgc"); print r;`

    
    
- [X] BinaryOpExpression
- [X] RangeStatement 
- [X] BinaryAndOrExpression
- [X] IfStatement
- [X] BinaryExpression
- [X] CallExpression script functions with precopile 
- [X] ForStatement
- [X] WhileStatement
- [X] ReturnStatement 
- [X] ContinueStatement
- [X] BreakStatement

### 0.9c: 

- [X] CallExpression parameter Bug:
```
> fn jo p; print "jo" p;;
> debug::bc "jo";
Segmentation fault (core dumped)
```
- [X] lot of optimize .. i got the speed back but i do not now where i lost it, so...

### 0.9d:

- [X] AssignOPStatement (not with direct path!)
- [X] Global variable lookup and cache the pointer (fast! :))
- [X] CallExpression fixed to lookup and cache the pointer (bytecode save)
- [X] ObjectFieldExpression with cache value

- [X] BinaryInlineExpression
    - [X] local 
    - [X] others to "slow" a = a + 1

- [X] MethodExpression 
- [X] Assignment does not set a value !!!!! print (a = 10) 
- [X] UnaryMinusExpression
- **2026-10-21**:
    - Now the StarField.13 runs in BC mode :) => ~ 315FPS (Interpreter does this with ~200FPS)
    - var test global (O2, null output): 1.729u 0.002s 0:01.74 98.8%     0+0k 0+0io 0pf+0w
    - var test local (O2, null output): 1.765u 0.000s 0:01.77 99.4%     0+0k 0+0io 0pf+0w
    - lol as fast as the local register .... but it's only one variable so no lookup time

- [X] BIG problem when precopile functions it does not know anything about the globals!
    
    

- [X] BUG recursive Function call: scripts/testBC_fibo.13 cause crash! - it crash at FIRST call?!
    - `debug::bc 'global num = 5; fn rec i;print i;i--;if i > 0; rec i; else return i;; end; print "i=" (rec num)'`
    - not only recursive! - is it the return ?! or the print call
    - its the return - it does NOT push a value!? it does ... but where is it 
    - THIS WORKS:`debug::bc 'global num = 5; fn rec i;print "inside" i;i--;return i end ;(rec num)'`
    - THIS WORKS: `debug::bc 'global num = 5; fn rec i;print "inside" i;i--;return i end ;print (rec num)'`
    - THIS CRASH: `debug::bc 'global num = 5; fn rec i;print i;i--;return i end; print "i=" (rec num)'`
    - OK i think the stack gets corrupted the the "i=" is cutted?!

    - ***REWROTE CALLSTACK!!* needs to be optimized
    
- [X] speedup callStack - hold a pointer on current locals !! << BUT did not speedup!
- [X] BinarySingleRightOpExpression (OP_NOT)    
- [X] TernaryExpression

- [X] ForEachStatement << this should enable StarField2 :)
- **2026-10-21**:
    - Now the StarField2.13 runs in BC mode :) => up to 500 FPS ... 400-500 ...

- [X] compile return false but it not handled anywhere!!!! << CRASH!!

- [X] KeyValueExpression
- [X] garbage collection when callStack is removed 

- [X] ArrayVariableExpression << this should enable StarField 5
    - StarField 5 => about 500 FPS :)

- [X] AssignStatement ( compileAssignment )
    - [X] VariableExpression local and global 
    - [X] ObjectFieldExpression with cache
    - [X] ArrayVariableExpression
    - [X] MethodExpression
    - [~] VariableReferenceExpression
    - [~] VariablePointerExpression

    
- [~] VariableReferenceExpression
- [~] VariablePointerExpression

- [X] IncludeStatement << tricky ?! or the same as a BlockStatement
    - Not sure what happen to my Function patching ?! 
    - Not sure how to skip the execute ... 
    - maybe it's easy lol it does a ParseScriptToAST at parse time so i should
      only handle the execute?! 
    - tested works fine

    
---
## CRITICAL BUT LATER:


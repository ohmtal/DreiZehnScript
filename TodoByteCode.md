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

- [ ] BinaryInlineExpression
    - [X] local 
    - [ ] global var - calculate wrong! i guess my new optimized code ?! 
    - [ ] others hackfest via expr pointer ^^ 

- [ ] AssignStatement ( compileAssignment )
    - [X] VariableExpression
    - [ ] ObjectFieldExpression
    - [ ] ArrayVariableExpression
    - [ ] MethodExpression
    - [ ] VariableReferenceExpression
    - [ ] VariablePointerExpression

    

---
 
- [ ] ReturnStatement - break_POPCount, break_Exit_OP
- [ ] ContinueStatement
- [ ] BreakStatement

- [ ] CallExpression script functions 
- [ ] VariableReferenceExpression
- [ ] VariablePointerExpression
- [ ] ArrayVariableExpression
- [ ] MethodExpression
- [ ] ForStatement
- [ ] WhileStatement
- [ ] ObjectFieldExpression
- [ ] TernaryExpression
- [ ] ForEachStatement
- [ ] IncludeStatement << tricky ?! or the same as a BlockStatement
- [ ] KeyValueExpression


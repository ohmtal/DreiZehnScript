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

### 0.9e: 
- [X] moved compiler from enviroment to own source file an splitted patch
- [X] ./DreiZehn scripts/bcLoader.13 test_Garbage << core dumped!
    add a function for that have enough redundant code! ==> _var_pointer_value(fieldValuePtr, PEEK_STACK(0));
    Garbage collection on: 
    - [X] ObjectFieldExpression with cache
    - [X] ArrayVariableExpression
    - [X] MethodExpression
   
- [X] flat locals with big init size 

- [X] globals have prio at the moment but locals should: 
    `debug::bc 'global i = 10; fn foo i; print i;; foo'`
    should be fixed in parser .. on AST interpreter it works correct .. 
    yes it works correct because it does a recursive lookup but mGlobal is here 
    set too.
    `} else if (Globals::isGlobalVar(id)) { ` this does a lookup find it and so 
    it set it to global. usually ok but i must check if i'am in another scope and
    it's an assign. It's called from ` else if (FunctionMap::IsFunction(nameTokenSymbolId)) {`
    Added here: I should also check peek is DreiZehn::TokenType::Assign
    ```
               } else {
                // we got a variable
                bool isGlobal = peekOffset(-2).mType == TokenType::Global;
                currentExpression = std::make_unique<VariableExpression>(nameTokenSymbolId, isGlobal);
            }
    ```
    
    Warning there is a other creation:
    
    ```
              bool isGlobal = peekOffset(-2).mType == TokenType::Global;
            return std::make_unique<VariableExpression>(nameTokenSymbolId, isGlobal);
    ````
    
    it's still tricky so i must build a frame system to check the scope for 
    variables to make it work also with bytecode. so it lookup the local variable 
    first if global is not explizit set. 
    
    ==> Added ParserFrame ... was not in good mood hope my tests was fine....
    
- [X] gerneric field cache path when caching is not possible. 
    
    
### 0.9f: testing and activate

 Globals::ByteCodeRunning

- [X?] bad && funny!  ValueObject::onMethodCall use this =>  Value CallScriptFunction(Environment& env, const ScriptFunction* sf,std::vector<Value>& args ); used and byte code call this at OP_CALL_OBJECT_METHOD fallback to AST-Interpreter bad idea 
funny is, that it works!
    ==> Heavy Metal ByteCode Patching! 
    
- [X] to get math faster remove string + string !!! and add an extra token for this
to bad i have use it in many scripts ^^  i may use SPC torque like do not want the 
'.' like php. python use also the '+' and lua '..'
    ==> `..` and `.=`
    - [X] AST-Interpreter
    - [X] ByteCode
    - [X] update ALL script .. may missed some statements
    - [X] remove +=
        - [X] AST-Interpreter
        - [X] ByteCode
    - [X] remove +
        - [X] AST-Interpreter
        - [X] ByteCode        

    - first tests .. i missed something or it's not faster ?! :(
        
- [X] run and eval should be checked then 

- [X] condional activate to run it without debug::bc
    default is IP(Interpreter) use -ip for interpreter and -bc for bytecode

    
- [X] BUG: something is wrong with locals or ARRAY or my change locals to outside    
Something is really wrong with [] access. Using it on new BaseFlux13 is cause crash 
with dangling pointer!!!!
```
template = Array::new "point" => 0, "color" => 0, "fill" => 0
Rects = Vector::new; Rects->fill 2 template
foreach item Rects; item[0] = (Vector::new 1 2 3 4);;
--
foreach item Rects; print item[0][0];;
--
foreach item Rects; item[0][0] = 5;;

```
THIS!=> OK here it works ... i Guess it's another garbage collection story!!! 

template = Array::new "point" => 0, "color" => 0, "fill" => 0
Rects = Vector::new; Rects->fill 2000 template
foreach item Rects; item[0] = (Vector::new 1 2 3 4);;

I first wanted to add garbage collection to a new Tick void or breath but now
i check the creation time 

- [X] compilePtr  (not tested!)
    - [X] VariableExpression local and global 
    - [X] ObjectFieldExpression with cache
    - [X] ArrayVariableExpression
    - [X] MethodExpression method is really slow so do i need it ?! 
    

- [X] replace assign on inline math 
    - [X] TEST VariableExpression local and global 
        - [X] OP_INC_PTR
    - [X] TEST ObjectFieldExpression with cache
        - [X] OP_INC_PTR
    - [X] TEST ArrayVariableExpression
        - [X] OP_INC_PTR
    - [X] TEST MethodExpression

    
    
- [X] change inline OP to also do it directly like the local!!! 
    - [X] changed to load by symid
    - [X] Bytecode push pointer
    - [X] Bytecode load 

    



    
## CRITICAL BUT LATER:
### ??: host reload

- [ ] inject code while running 
    - debug::bc and run call complete new VM this is bad while the VM is running 
    function calls must be patched in and such.
    - cant use inlude as a replament for run since i does only accept a LiteralExpression
    as filename. 
    - CompileExec encapulate BytecodeChunk chunk; and CompilerScope scope; so it
    cant be accessed from outside. it does:
        - compile
        - patch
    - IncludeStatement does:
        - in parser: IncludeScriptFile
        - in compiler: only compile the body it it should be executed and 
        patch in a jmpOverMarker via OP_JUMP_IF_TRUE
        - so it register the functions at parse time which are patched by compile
        exec. my patch system is so crazy ;P
    - to get a run working:
        - 1. ParseScriptToAst => register functions
        - 2. Patch All bytecode again i guess mFunctionPatches should not by 
            cleared after intial patch - adresses may change after an include 
            like in scripts/main.13
        - 3. problem! every new injected include add new byte codes at the end.
            functions are always called by adress - which is patched. I think
            about a chunk stack but merge while running is not a good idea when the
            byte code change which is currently running . 
        - 4. so hot reload is not possible with DreiZehn bytecode ?! ... no solution
            so far. Did i break this in ElfScript too with my inline function 
            runner ?! ==> 
        - 5. Solutions:
            - a) every "run" must be added to the current bytecode no matter
                if it was added before and then the the complete byte code must
                be patched.  ==>  **but what is if the main script is hot reloded ?!**
            - b) run everything in the ast interpreter and load critical stuff as
                bytecode as i did with debug::bc. 
        - @a ==> save the function pointer NOT the jump adress and var_count!!!
                 and yes i must add it at the back. 
    - [X] changed adress and varcount patches to pointer patch 
    - [ ] attach new code at back ?! what about a jump to this code ? how to come 
            back... with functions it would work  but what aber the "normal" code
            
---



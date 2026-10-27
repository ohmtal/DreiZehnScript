# DreiZehn Todo

## 0.2

- [X] fix Identifier and Keywords can be delimited by a '.' or ':'
- [X] neg numbers and fix '.' somewhere outside a number  
- [X] fix Garbage collection to current scope, using a current scope global env 
It needs to be done in "end" 
- [X] add preprocessor with ';' break to lines
- [X] added multiline statement parser and fixed issues with undetected syntax errors - endless loop
- [X] while or similar while(true) loop

## 0.3 

- add some basic SDL3 bindings ;) Test if i can use my ElfScript macros - but i 
don't have  PoD Types here at the moment. 
- [X] setup project Simply added to current CMake
- [X] add some basic bindings
- [X] fixed if statement broken after while inserted
- [X] added getDouble/getInt for auto type convert
- [X] BinaryExpression::evaluate uses getDouble
- [X] Added else and finally fixed If as BlockStatement - run in a own execute !! .. variables ?!?
- [X] "!=" as compare 
- [X] add a constant system ...
    - true and false added in core
- [X] write the starfield demo in DreiZehn
- [X] test params  => if (math.random 10 20) != 4 print "huhu" end
- [X] need: ||, &&, <=, >=, >>, <<, |, &
- [X] Pointer method calles with Array '->' \o/
- [X] rewrote RunScript .. 
- [X] added Symbols Table for fast lookup variables/functions/constants ....
- [X] optimized a bit found 3 sek on test_var bench ;) 

## 0.4
- [X] VariableFrame for variables and garbage collection
- [X] ValueObject type registry (gUserObjectTypes / registerUserObjectType)
- [X] When a new String is set the old should be deleted!!! ..
    => added an mAssigned flag
- [~] add delete method to ALL object.  .. onMethod must call the parent 
    => let the garbage collection do that - i added the assigned flag. 
- [X] Bytecode 
    - [X] change the direct threading like it is in ElfScript
    - [X] While i port it use the new VariableFrame 
- [X] Better Garbage collection and array also keep track of object assignment
- [X] Before I continue with ByteCode I'll optimize the current code i guess i can get
under 15 sec and beat python here without bytecode ;)
    - [X] Operator ++, --, +=, -=, *=, /= 
    - [X] other BinaryExpression to switch case
- **12.478u 0.002s 0:12.52 99.6%    0+0k 0+0io 0pf+0w** 

## 0.5
- [X] changed all static calls to ":" like math:random or Array:new... so i can use Dot for fields!

- [X] object fields Vector3 example: `v = Vector3.new 1.0 2.2 3.3`
    - [X] using v.x instead of a method -> caller . At the moment the lexer add the '.' to the variable name
        - [X] onGetField => `v = Vector3:new 1 2 2; print v.x v.y v.z` => `1.000000 2.000000 2.000000`
        - [X] onSetField
                TODO: v.x = 1 Syntax-Error: Unexpected token 'Assign' (v.x = 1:1)

    - [~] fix method expression:
        - this only return the first value: `print v->x v->y v->z` => 1.0
            This is because the method call v->x eat v->y v->z .. thats where my syntax kick my ass.
        - The method may tell how may arguments it did consume. 

- [X] Field Assign
- [X] Field Inline MathOp (++/--)

## 0.5c: 
- [X] math fixed: order of operations
- [X] added `ForRange i 10` is the same as in C: `for (int i=0; i < 10; i++)`
- [X] fixed bug in function (fn) parameter variable scope - and force parameter as in local scope
- [X] added optional "Fenster" lib - had to modify fenster.h a bit -. It's handled as Object with methods. 
- [X] ported Fenster Drawing Example to DreiZehn 
- [X] bug on "-" => print (10-1) is 0! , print (10 -1) is error and  print (10 - 1) is 9, while print(10+1) is 11 ok 
    ==> print (10 -1) is still an error but thats ok i guess. 


## 0.5d:
- [X] renamed static / tool calls. Now and finally separated by '::'
- [X] Fenster added some: KEY_ COLOR_ constants
- [X] Added some help commands 
- [X] add core::breath to enable console while running a loop 
- [X] Added StringTable and set all variable strings here with new Value Type StringId
- [X] Since main loop input is non blocking i need a sleep
- [X] added Not `!` or `not` and Module `%` - when both are float it's an fmod
- [X] added `;;` for lazy end
- [X] added export to Fenster 
- [X] changed the register system to only one line per prop
- [X] added one line func for method or field match (matchMethod/matchField)
- [X] Fenster added to the Register System 
- [X] Fenster audio
- [X] Added XAudio

## 0.5e
- [X] separate StringFunctions
- [X] add a toString in ValueObject 
- [X] core::getType

## 0.6a
- [X] moved src to demo and engine to src and add an DreiZehn.cmake file
- [X] Add Base Nodes to clean Enviroment::execute **hope this does not slowdown**
- [X] :( cleaner code cost me about 2 sec in test_var/test_field - i think it's the assign ? 
- [X] found my 2 seconds and got one more

## 0.6b
- started at test_var time:  11.218u 0.012s 0:11.28 99.4%    0+0k 0+0io 0pf+0w
- [X] LiteralExpression :: optimized - value is cached after evaluate! :D - 3sec :)
        => test_var: 8.306u 0.002s 0:08.33 99.6%     0+0k 0+0io 0pf+0w
- [X]  BinaryOpExpression::evaluate optimized using switch - not faster. 
        => 8.383u 0.000s 0:08.41 99.6%     0+0k 0+0io 0pf+0w
- [X] nested if / else: `if (i == 1) print 1; end else print "not"; end`
            only this works: `if (i == 1) print 1; else print "not"; end` and cant
            be nested. Also added mIsImplicit so only one end must be set :)
- [X] string format str::format `"Hello %s" "World"` 
- [X] string OP: 
    - [X] '==' '!=' BAD:  print ("UHU" == "HU") => 1 
    - [X] also '+' '+=' 
- [X] fixed Lexer `t=0;print(t-2);` had problems with the "-" which is used for neg numbers
- [X] parse '#' in lexer as comment - it also can close a comment ! 
- [X] Multiline Statements !! :) ==> Backslash as line break ignore 
- [X] added short if ? : 
- [X] fixed math order again and added BitXOr to order


## 0.6c
- [X] BUG: my mIsImplicit addon `not` became bad `!`
        - cant add an if inside an else statement!!
    
- [X] need continue!!! 
    - [X] while
    - [X] for
    - [X] range
- [X] need an `elif` \o/

- Objects 
    - [X] Object Methods - have to rebuild the call name -- which is not very fast!
        - [X] user defined methods like: `fn Array::foo this; print this; end;` called with `myArr->foo`
        - NOTE: All objects must call =>  `return ValueObject::onMethodCall(methodId, args, ret);`
        - [X] add *class* to call: `myArr->setClassName "SuperArray"`; `fn SuperArray::foo this; print this; end;` called with `myArr->foo`
        - [X]  cache the methods ! 
    
    - [X] Object dynamic fields - can be cached in a table 
        - [X] IMPORTANT ALL OBJECTS MUST CALL Parent:
            - bool onGetField(uint32_t fieldSymbolId, Value& ret) 
            - Value* onGetFieldPtr(uint32_t fieldSymbolId)
            - bool onSetField(uint32_t fieldSymbolId, const Value& value)
            
        - [X] add a map for the field symid/value 
            
    - [X] add a base ValueObject Class Object::new
    
- [X] Bug in assigned counting up like hell (module/RayFirst ) => #29 [0x55fddc2524c0] assigned: 1293 type:1 Object
        was the "this" in function call - i decrement it again after call return 
- [X] core::gc does not remove the not assigned objects ?? 

- [X] create ValueObject->isScriptMethod => need also to do the same as the calling code  so I should add a function 
    - [X] dump should also list the registered scriptMethods!

    
## 0.6d     
- [X] bug return without value: __return_value__ << return 0 if not expression is found
- [X] Fenster Scale 
- [X] help print constants 
- [X] Fenster PixelBuffer object 
- [X] GC: on local var and function parameters: gc count up but is not decremented when frame is deleted 
- [X] alot of stuff to FensterFunctions added

## 0.7a

### Attepmt: instead of hack in array[] with dummy variables try to add an assignment on methods!

- `a = Array::new (Object::new) "cow"`
- `a->at 1 = "Jo cow!"`
- `(a->at 0)->myMethod`
- `(a->at 0).foo = 1`
- what about a this `[a->at 1]="YES!"` - maybe
- object must return a methodValuePtr to get it work or nullptr if not supported 
- if i add the [] for this it could be parsed as a Variable Method Expression , then i also add a Variable Field Expression ?!
- At the moment the Field is hacked in special and does not allow nesting
- No idea if I get this work but i like it more then the `arr[0]` expression because i had affed arr____0 var for this .. 
- writing to a method directly would be much cooler using the ArrayObject or something else which would support it
- or to keep it simple it should be this => `(a->at 0) = "freitag"` parser must except this so the runtime can 
    handle it. 
- same for getter. `a = Array::new Object::new; (a->at 0)->dump; and not tmp = a->at 0; tmp->dump`
`(a->at 0)` return the Value to it also can accept the method call or field access ... 
- i maybe get in trouble with my fast op variable lookup then, because it's runtime stuff not compiletime 
- **FINALLY** `(a->at 0)->dump` does NOT work! but `a[0]->dump` does :) and i renamed Array to Vector
- **FINALLY** `(a->at 0).x = 0` does NOT work! but `a[0].x=0` does :) and i renamed Array to Vector
- **FINALLY** `a = Vector::new 0; a[0] = Vector::new 10 20` and `print a[0][1]` does work! 

- [X] parseLine change from if to switch
       - became slower ?! => 8.897u 0.005s 0:08.93 99.5%     0+0k 0+0io 0pf+0w 
       - NOT i just checked 0.6 pre switch it's same speed i did add a break somewhere before! 
- [X] Rewrite of =, ++,--, !, *=,... 
    - [X] added a evalute to get the pointer virtual Value* evaluatePtr(Environment& env)
        - [X] VariableExpression
        - [X] ObjectFieldExpression
        - **NOTE** more to come if it works! 
        
    - [X] add left expression and change the parser:
        - [X] BinaryInlineExpression (++/--)
            - [X] added to isMathType
            - [X] added to getPrecedence
            - [X] Definition
            - [X] evaluate
            - [X] parser (parseMath!)
        - [X] Assignment     
        - [X] OPAssignment     
            
    - [X] Benchmark .. will be slower i guess .. << NOT 
        same as before: 8.947u 0.004s 0:08.97 99.6%     0+0k 0+0io 0pf+0w
    
- [?] Else is broken ? => `b = !b; if !b; print "is not"; else; "it is"; end; print b`
    => works `a = !a; if !a; print "ja"; else print "not not"; end; print a`
    
- [X] fixed variable and field must return a new Value(0) in getPtr if not found!
    
- [X] now we are in ... add ptr return to Array.at 
    - [X] add a new method caller => onMethodCallGetAssignPtr
    - [X] MethodExpression << Value* evaluatePtr(Environment& env) override;
    - [X] Test: `a = Array::new 10 20;(a->at 0)++;print (a->at 0)` 
    
- [X] did this also: `a = Object::new; a.o = Object::new;`    
    - I guess i need a extra parser like the parse math ? 
    - [X] field: `a.o.x = 1` << Syntax-Error: Unexpected token 'Dot Object field access' (a.o.x = 1:1)
    - [X] method: `a.o->dump` << Syntax-Error: Unexpected token 'Arrow Object Method call' (a.o->dump:1)
    - YAY!
    
## SOLVED in 0.7a :) => Variable problematic 

At the moment it uses flat assignments Identifier = | Identifier.field = . this allows fast setup using Symbols
varid and varid.fieldid. If I change this to a variable expression i need to parse it on runtime which is much 
slower than on compile time. So at the moment only flat variables are allowed and i dont get the array assign
working. I did add it as a "dummy" variable which is not the best idea anyway.

    
## 0.7b    
    
- [X] Vector, Array, [] 
    - [X] rename VectorObjectFunctions => PointVectorFunctions
    - [X] copy ArrayFunctions to VectorObjectFunctions
    - [X] ValueObject add getIndexPtr for ArrayVariable Expression 
    - [X] Add this to Vector 
    
    
    - [X] ValueObject: virtual clone where  the dynamic fields and className
          is cloned when mSupportClone is true and it's called from child with 
          ValueObject::clone 
```
foo = struct "x" "y" "z"
v = Vector::new
v->fill 10 foo
forRange i 10; print v[i];;
Struct [0x5576cc6a4380] 
Struct [0x5576cc6a4540] 
Struct [0x5576cc6a4720] 
Struct [0x5576cc6a4900] 
Struct [0x5576cc6a4ae0] 
Struct [0x5576cc6a4cc0] 
Struct [0x5576cc6a4ea0] 
Struct [0x5576cc6a5080] 
Struct [0x5576cc6a5260] 
Struct [0x5576cc6a5440] 
```
**Also GC is fine**

- [X] fix memleaks fenster

- [X] forEach with ValueObject iter call `v = Vector::new 10 20; foreach item v; print item; end`

- [X] why is it slower ? i changed math a bit , when i worked on bytebeat << this ?
    yes it's the math. after long time trying to make it faster - i give up here!
   AssignOPStatement maybe changed like I did on normal math but i guess it's not faster

   
   
## 0.7c
- [X] optimized iter loops :)
- [X] small optimize register ReturnValueSymbol once
- [X] removed string concat from += !! tryed to optimize it but this is slow!
- [X] Added FunctionMap::ModuleRegistry using `modules` and `import`
- [X] Syntax complete for 1.0 ? 
    - [X] added some examples in examples/
    - [X] inital handbook and examples

    
## 0.7d 
- [X] import does not work when it's running :
    lol the function is looked up while parsing so the Interpreter do not 
    get a function and it's intepreted as unknown variable!
    The lookup on parsing is the best for speed so my Module system is useless!
    ===> the whole script needs to be parsed again since it's in the wrong
         token type or i need to change the preparser ... looking for import 
    THIS => add `#import` to lexer/parser and call it directly , as function was nice but 
            it sucks because of the lookup before the rest of the tokens is generated!!!
- [X] `#` fully to lexer/parser and remove it from preparser  (ScriptLoader)    
- [X] add a `#define name value` for constanted also in lexer/parser to setup constants 
- [X] update scripts with import
- [X] move pixel_buffer to toolbox/
- [X] printf errorf to a overwritable class or add a handler 
        ==> LogHandler
- [X] cleaned up stuff
- [~] Variable Cache attepmt - was fast but did not work so well.... 

## 0.7e
- [X] working on "str::" functions - disabled String object ?! 
- [X] -4 and !true must be capsulate by () ,,,mhh
    - not fixed
    - `-` must be encapsulate 
    
## 0.7f    
- [X] added FlowStatus with return value .. good boost and fixed bug :)


## 0.7g
- [X] Value Reference
    - [X] '&' Tokenizer / Parser => `foo = 1; print (& foo)`
    - [X] '*' Tokenizer / Parser => `foo = 1; ptr = & foo; print ptr (*ptr)`
    - [X] `foo = Struct::new "x" "y"; ptr = &foo.x; *ptr = 99; print foo.x`
    - [X] StarField3.13 with pointer! << not faster - a bit slower ! than StarField2
    - conclusion: not faster but funny

## 0.8a
- worked on handbook and added buildin functions description
- added copyright note zserge/fenster because of used parts in PixelBuffer
- replaced the fill algo with a fast save fill_scanline 
- a bit cleanup
- [X] GarbageCollection as Singleton Class (removed from VariableFrame)
- [~] VariableFrame pool --- **rolled changes back it was slower than before!!!!!!**
- [X] VariableExpression cache 
- [~] Object Field cache **only works on dynamic fields!!!!**


## 0.8b
- [X] guarded all Register functions 
- [X] nailed gcc Release build to -02. Some tests run slower but StarField2 for example result in much better FPS (260 instead of 230)

## 0.8c
- [X] FileStream base also for stdin/stdout :)
- [X] refactored dynamic field and added getFieldCount, getField INDEX, setField INDEX value
- [X] FIXME Auto GarbageCollection is disabled at the moment !  
- [X] FIXME Variable scope - i guess i broke it somewhere - look at scripts/test_Scope.13 !!!!
    - i did at Variable cache!! , removed gCurrentFrame!!
- [X] FIXME something is still wrong with Assign ! scripts/test_Scope.13 at the end it should be cleared
    - it works but since my change the childs are not removed directly so i have to call it more than once
- [X] test in text \\n i added it but not sure it it fail on my print!    


## 0.8d - the global var change 
- [X] I need to change the variable scope .. 
    - [X] since i fixed the scope bug, StarField2 is slower 
            ==> optimized the getCurrentFrame a bit 
    - [X] check if ONLY script functions run in it's own Enviroment << YES
    - [X] changed all loops to current env !!!!!!!
    - [X] if so we can go ahead and add "global" to lexer and Expression
    - [X] modify the Variable Frame Lookup to only toggle main or current
    - ==> funny this is not faster :/
- [X] fill unset parameter with 0 at function call
- [X] fixed foreach issue with assigned
- [X] changed Varableframe Value to unique_ptr
- [X] add Arrow and Dot to expression to also use a constant or methodcall 
    - `o = Object::new; o.struct = Struct::new "x" "y";`
    - [X] Dot: `((o->getField "struct").x)++; print (o->getField "struct").x;`
    - [X] Arrow: `(o->getField "struct")->dump`
    
- [X] test why define constants sometimes fail (prio ?! ) 
      happend on modules/RayFirst.13 screenWidth screenHeight
      , note: FensterShooter was loaded before 
      ==> test.13 test says define overide varable   
      
- [X] added optional "=" in `define foo = "bar"` easier to change from a var ;)


## 0.8e - Inline Field cache :D
- [X] fieldcache again ... i want this ;) 
    - works ok in most tests but on FensterShooter - using object :
        - i have a global object Game with className Game
        - key space is pressed Game::reset is called 
        - this.gameOver = false ; does not change the field!
            - Object [0x55ec88cf05b0]   gameover = 1
            - RESET  Object [0x55ec88cf05b0]  lastSpawn= 1791852217955.000000 GameOver: 1 
    - [X] changed dynamic fields also to unique_ptr ! this fixed it :D


## 0.8f  
- [X] moved Parser header source to cpp
- [X] moved Lexer header source to cpp
- [X] print  0xff0000ff ==> -16776961 . its correct but bad 
      also   print  float::cast 0xff0000ff does! should added as uint32 
      i cast it as double!
- [X] added desc map and parameter to RegisterFunctions 
- [X] coding and testing common tools
- [X] added core::getEnv and core::fetchEnv to get enviroment stuff
- [X] added core::time, core::date and  core::dateutc
- [X] Comma hack to separate parameter at a function call. this works fine but when it is a function call WITH 
        parameter inside a function call - it still need the () !!!!! I dont want to change it to a foo(a,b,c)! i like my
        foo a b c  or now foo a,b,c
- [X] added a third value to `for` the step.
- [X] added `include filename`
        
## 0.8g
- [X] added noExec parameter to `include filename` 
- [X] moved addons from ext/ and src/functions/ to addons/ and added cpp instead of h
- [X] linenoise.c does not work on ms
    => changed to cpp-linenoise in a thread
- [X] made linenoise optional 
- [X] * try to fix msvc build * 
    - 1. disable all addons in cmake
    - 2. start hunting 
- [X] Added KeyValue with => constuctor for Array and Struct :)
- [X] Added ArrayObject / key value table 
- [X] Array: foreach array keys ...but how ..
    - php does foreach ($array as $key => $value)
    - lua for key, value in pairs(my_table) do
    - python keys = my_dict.keys() ... for key in my_dict:
    - at the moment it can be done with:
        `forRange i a->getFieldCount; print (a->getFieldName i) a[i];;`
      this is because Array inherit from ValueObject's dynamic fields
    - I could add a method ->getKeys which return a array ..
    - THIS=> `a = Array::new "x"=>10, "name"=>"hello"; a["adress"] = Array::new "street" => "Elmstreet", "zip" => 12345; forRange i a->count; print (a->key i) "=>" (a->value i);;`
    
- [X] if args on AND (&&) are all evaluated also if the first one is false !! 
    
- [X] check where i added an object with count and "_" values as an Object =>  instead it can be a array now! 
    - [X] sys::args << need to change all script .. i added it alot .. lol
    - [X] fetchEnv
    - [X] exec result - it's one dimanonal so i simply add the index as string too
   
- [X] Script Tree to c++

## 0.9 Bytecode
-  AST-Interpreter: StarField2 is really slow now dropped down from 340 in 0.8e to now 285
but why ?! - i added array ...and moved the SDL3 Functions to cpp?! I just fetched 0.8
it's the same problem.
    - [X] is it the SDL3 move to cpp ? => NO
    - [X] i switch to linenoise cpp => NO
    - NOTE: lol since yesterday 500ms lost on scripts/testBC_simplevar.13 
        did if have a lucky binary yesterday ? 
        
- [X] TOO much is inlined! 
        
- [X] DREIZEHN_DEBUG flag! 
- [X] DECISION pure byte code ot ast/bytecode hybrid ==> pure bytecode which caching ptr :P 

- [X] **removed**: VariableReferenceExpression and VariablePointerExpression!
    While it was funny, it make no sence! ... maybe a comeback when i can do thinks like
    `foo = &(vec->at 0)` which was not possible. It was not faster and i do not port this
    to ByteCode!
    
todos listed here: [TodoByteCode](./TodoByteCode.md)

- 0.9e: Wooot! found the handbreak in Ast-Interpreter  
    - test_var: 2.631u 0.000s 0:02.63 100.0%    0+0k 0+0io 0pf+0w
    - test_field: 3.149u 0.008s 0:03.16 99.3%     0+0k 0+0io 0pf+0w
    - test_fibonacci: 2.388u 0.004s 0:02.39 99.5%     0+0k 0+0io 0pf+0w
    - StarField2: 330-350 FPS
    - *NOTE:* -O2 is still faster than -O3

- 0.9f: more Optimize (replace CFunction call vector with span (c++20))
    - tests with **-O2 and > /dev/null** and best of three
    - test_var: 2.341u 0.005s 0:02.35 99.5% 
    - test_field: 2.936u 0.004s 0:02.94 99.6%
    - test_fibonacci: 2.382u 0.003s 0:02.39 99.5%
    - StarField2 (bench): 6.878u 0.199s 0:07.30 96.7% 
    - i had test_var down to 2.1 and field down to 2.7 no idea what it make it slower again 
    
    
    
- [ ] Testing BaseFlux13
    - [ ] Tools::LogHandler should be added to Init
    - [ ] eval is a bad idea to call events like onRender
        - on Interpreter i can simply use => FunctionMap::CallScriptFunction
        - on Bytecode it's compiled in !
            - CallExpression::compile create the chunk with a placeholder for the 
              FunctionPointer
            - I may compile a new call with a single chunk, and patch it
            - i also have all parameters as LiteralExpressions in the AST
            - it should look like
                - for each param:
                    - OP_CONST 
                    - U32 constIdx (pushed as chunk.addConstant(this->evaluate(env));)
                - call
                    - OP_CALL_SF_FUNCTION
                    - U32 argCount
                    - pointer to ScriptFunction
                    - IF NOT RESULT IS REQUESTES: OP_POP_STACK
            - example flux::init is 5 bytes
                - OP_CALL_SF_FUNCTION
                - 0
                - 0x... pointer 
                - OP_POP_STACK
            - example flux::OnMouse x y button isDown << 13 ( 4 * 2 + 5) bytes 
                - chunk.addConstant x, patch pos + 1 
                - chunk.addConstant y, patch pos + 3
                - chunk.addConstant button, patch pos + 5
                - chunk.addConstant isDown, patch pos + 7
                - ONCE! patch pointer: pos + 10
                
              
    - [ ] Objects
        - [ ] mClassName is a String not a Symbol?! 
        - [ ] need ->clone method on ValueObject


##  SDL3 parser .... ongoing process .... wait until ByteCode is done !! 

- [X] common/FileTools.13
- [X] common/JSONParser.13
    
- [ ] sdl3 clang header tool 
    - [X] test and move into DreiZehn tools
    - [ ] create a json parser using DreiZehn :D 
        - [X] lexer / tokenizer
        - [X] consts
        - [X] structs (basic)
        - [X] write a reusable load/save  file - should be in common scripts (new folder) 
        - [X] change the parser to recursive to handle multiple lines 
        - [X] remove stdin here and load the definition from file it does not change so often!
                This enable my console again - i guess.
        - [X] separate the Tokenizer 
        - [ ] load hints 
        - [ ] write a sourcefile  
        - [ ] functions
            - adding  hints for function parameter to know what is an array like the points:
            - `bool SDL_GetRectEnclosingPointsFloat(const SDL_FPoint *points, int count, const SDL_FRect *clip, SDL_FRect *result);`
                - "points": { "passing": "array", "size_by": "count" },
                - "clip": { "passing": "in_ref" },
                - "result": { "passing": "out_ref" }}
            - OR: simply skip :P ... not sure
            


---

## still missing:
- [ ] ByteCode: hot relad and run!
- [ ] Better Error messages: Parser i added linenumber to base AST Node but it's not used anywhere !
    - most terrible error is missing end !! << need to backward check the start points 

- [ ] remove fenster audio and implement MiniAudio instead 
- [ ] FileStream ->getError and test modi 
- [ ] from hex: int::cast "0x37" ==> 7 , note: float::cast works but depends on compiler! 

## maybe:
- [ ] Handbook: CheatSheet for basic syntax and functions.
- [ ] while `foo = "abc"; print foo[2]` is working foo[2] = "a" is not. 
      ok yes, it is only working with object which support it like Vector
      not sure if i want do change this

## maybe maybe
- need a die command ?! 
- finish Raylib importer << stuck at structs inside structs and pointer returns 
- validate if there is a usable SDL3 auto bind script parser 
    - maybe https://github.com/TerensTare/SDL_parser
    - or https://deepwiki.com/ppy/SDL3-CS/2.1-c-binding-generation
- UserFunction object to call them like a lambda from a list... 
- switch / case
- Objects: change the methodCall to Lambda !!!! so i can directly looked up

 
## aborted but still in code:
 
- [~] StructObject add a assign like {1 ,2 ,3} THIS SHOULD NOT override the 
        Objects content it should set the first 3 fields - unordered_map save? -
    - *ROLLBACK* RCurly/LCurly/CurlyArgumentsExpression - was nonsence! 
    - [X] vector mFieldOrder
    - [X] overwrite setfield/getfieldptr to deny new fields 
    - [X] Lexer {}
    - [.] parser + ast expression
    - [~] Value object method to support this 
    - [~] assign special handling 
        
    

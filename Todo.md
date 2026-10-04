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
don't have to PoD Types here at the moment. 
- [X] setup projecct Simply Added to current CMake
- [X] add some basic bindings
- [X] fixed if statement broken afer while inserted
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
- [X] changed the register system to only line per prop
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
- [X] BUG: my mIsImplicit addon not became bad !
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
    
- [X] not we are in ... add ptr return to Array.at 
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
    
- [ ] Vector, Array, [] and = { }
    - [X] rename VectorObjectFunctions => VectorObjectFunctions
    - [X] copy ArrayFunctions to VectorObjectFunctions
    - [X] ValueObject add getIndexPtr for ArrayVariable Expression 
    - [X] Add this to Vector 
    
    
    - [X] ValueObject: virtual clone where  the dynamic fields and className
          is cloned when mSupportClone is true and it's called from child with 
          ValueObject::clone (if overwritten)
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

    - [ ] StructObject add a assign like {1 ,2 ,3} THIS SHOULD NOT override the 
          Objects content it should set the first 3 fields - unordered_map save? -
          - [X] vector mFieldOrder
          - [ ] overwrite setfield/getfieldptr to deny new fields 
          - [ ] Lexer {}
          - [ ] parser + ast expression
          - [ ] Value object method to support this 
          - [ ] assign special handling 
          
    - [ ] foreach with ValueObject iter call 
        
    
- [ ] why is it slower ? i changed math a bit , when i workd on bytebeat << this ? 
     


## still missing:
- [ ] change printf errorf to a overwritable class or add a handler 

## maybe:
- [ ] Instead of hacking in slow array brackets i will add a **foreach**
    - [ ] foreach need a interator information from the Object:
        - bool support foreach
        - int count
        - getter with index 
- [ ] Bytecode continue .... 
- [ ] move more code to cpp because compiletime is raising - carefully do not break the speed
- [ ] UserFunction object to call them like a lambda from a list... 
- [ ] finish Raylib importer << stuck at structs inside stucts and pointer returns 
- [ ] validate if there is a usable SDL3 auto bind script
- [ ] switch / case
- [ ] Operator callback for Objects
    - [X] add OP "event" in ValueObject << simply insert script function symbol
 
    

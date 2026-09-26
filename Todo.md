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
- [X] ported Fenster Drawing Example to  DreiZehn 


## 0.5d:
- [ ] add core.breath to enable console while running a loop 
- [ ] multiline statement for example if..


# future ideas
- [ ] Bytecode continue .... 
- [ ] header only - for easy include 
    - Problem : CallExpression <> FunctionMap < ASTNode
    
- change printf errorf to a overwritable class or add a handler 


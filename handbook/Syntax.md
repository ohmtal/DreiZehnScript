# DreiZehn-Script: Syntax

## 1. Syntax 

- The ';' spearator is only needed if you write multiple statements in one line.

- Compare: 
    - Or: ||
    - And: &&
    - Equal: ==
    - Non Equal: !=
    
- Supported operations: 
    - Add: +, ++, +=
    - Sub: -, --, -=
    - Mul: *, *=
    - Div: /, /=
    - Bitwise shift right: >>
    - Bitwise shift left: <<
    - Bitwise Or: |
    - Bitwise XOr: ^
    - Bitwise And: &
    - Strings can be concat with `+` and compared with `==`/`!=`
    
- Keywords
    - if        Conditional if
    - else      Conditional else
    - elif      Conditional elif
    - fn        start a function definition
    - end       end a if,else,elif,for, forRange, forEach the short alias is `;;`
    - for       Start a loop `for i 1 10` count from 1 to 10 (inclusive)
    - break     break a loop 
    - return    return [value] from a function optional a return value
    - continue  continue a loop 
    - while     while condion - loop 
    - forRange  start `forRange i 10` count from 0 to 9 - alias forrange
    - forEach   start `forEach item myVector` iterate - alias foreach 
    - not       logical not alias for `!`
    - and       logical and alias for `&&`
    - or        logical or alias for `||`   
    - import    import one module `import Math` or `import All` to import all modules
    - define    define a constants `define HALF_PI 3.14159265358979323846 / 2.0`
    
- Note Object fields only allow assign Inline Operation (`++`) or Operation Assign (`+=`) is not 
implemented so far.
- If: `if i == 5; print "is 5"; else print "is not 5"; end` 
You always need a end at the end. Same for loops.
- Loops: 
    - for: `for i 1 2; print i; end` from to (inclusive) 
    - while: `i = 0; while i < 5; i = i + 1; print i;end;`
- Break: `break` the current statement 
- Contiunue: `continue` the current statement loop
- Return: break the current statement and may return a value : `return 4`
- Function parameters are simply added: `print "Hello World"`
- If you need to pass statements you can use: `print "Hello" ( 1 + 1 )`
- Static methods use usally "::" or you use the C-Name like i did on SDL3 binding test.
- Object constructors are with a big first captial like: `Array::new`
- libary function calls are all lower case like `math::randomf`
- methods on objects are called with a Arrow: `arr->pop` 
- fields on objects used the Dot: `vec.x = 10.0`
- fn: function definition: `fn hello param1 param2;print param1 param2; end; hello`

- Uncomment: 
    - Full Line at the start of a line: `#` or `--`
    - Inline `x = 6 # this is a inline commend # + 3` ==> x is 9
    
- NewLine The parser was inital designed to parse line by line. If you like to 
add a linebreak for one statement `if` for example use the Backslash `\` at the 
end of each line you want to continue. 

### When do i use a bracket : 
- On Math to force the order : `( x + 5 ) * 10` 
- To capsulate a function call: `print (myVec->at 10) player.x` 
This is imporant so the parser known when the parameter input of `->at` end. 
- To capsulate a negativ number in a function call: `print str::substr "NoteBook" (-4)`. Else math thinks you want to calculate "NoteBook" - 4

## 2. Data Types & Literals

The language automatically infers types during runtime evaluations.

```
# Integers and Floating-Point numbers (Doubles)
x = 5
y = 3.1415
color = 0x0000FF

# String Literals 
message = "Hello World"

# Objects 
obj = Object::new

```

---

## 3. Variables & Assignment

Variables are dynamically typed and stored in the active environment. 
Assigning a value updates the local scope. Variables are looked up recursive
in the stacked environment. 

```
speed = 100
multiplier = 2
total_speed = speed * multiplier

fn newSpeed
   x = 10       # was not defined before it's in the local scope
   speed += x   # the speed variable defined previous (global scope) 
   return speed 
end
```

---

## 4. Mathematical & Logical Expressions

Expressions support standard operator precedence, comparison routing, and recursive parenthesis grouping `()`.

```
# Math operations
result = (5 + 5) * 2 - 1

# Comparison operators (Return 1 for True, 0 for False)
is_greater = 10 > 5
is_equal = x == 5
```

---

## 5. Control Flow (Conditional Statements)

Conditional logic evaluates any non-zero integer or double as `true`. Block contents are executed sequentially.

```
if x > 5
    print "Value is greater than 5"
end

if status == 1
    y = x + 10
    print y
end
```

---

## 6. Loops & Iteration (For Statement)

The `for` loop introduces an isolated nested environment. Loop iterators are local and automatically cleaned up upon exit. Supports safe early termination via `break`.

```
# Simple increment loop (Start to End, inclusive)
for i 1 5
    print i
end

# Loop with early break conditions
for i 1 100
    if i == 5
        print "Target reached, breaking loop."
        break
    end
end

# forRange loop counts from 0 to 9
forRange i 10
    print i
end

# forEach on supported Objects like Vector:
foreach bullet Bullets
    bullet.y += bullet.speed
    if bullet.y > HEIGHT
        bullet.active = false
    end
end

```

---

## 7. User-Defined Functions and Methods

Functions parse their AST blocks exactly once and run them on demand inside a private variable scope. Parameters are bound dynamically at call-time. Supports immediate execution halts and expression pipe-backs via `return`.

```
# Function definition with arguments
fn calculate_bonus score factor
    if score < 50
        # Early exit with return value
        return 0 
    end
    
    result = score * factor
    return result
end

# Invoking script functions and storing the return value
my_bonus = calculate_bonus 85 2
print my_bonus
```

### Script Methods with NameSpace:

```
fn Player::init this
    this.x = math::random 0 WIDTH
    this.y = math::random 0 HEIGHT
    this.speed = 0.5 + math::randomf * 1.5
end

player = Object::new
player->setClassName "Player" 
player->init

```


## 8. Built-in Safety Features
* **Infinite Loop Prevention:** Parser-level locks intercept stalled index trackers and throw non-blocking compiler alerts.
* **String Memory Safety:** The lexer forces automatic emergency lookbehinds on unclosed string sequences (`"hello...`) to prevent state corruption.
* **Garbage Collection (GC):** Destructors walks through all dynamically tracked memory nodes upon exit to prevent memory leaks in the host C++ application. You can trigger a Garbage Collection with: "core:gc" and check it with "debug:garbage"



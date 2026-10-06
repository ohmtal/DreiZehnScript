# DreiZehn Script

A lightweight, modular scripting language built from scratch in C++. It was 
designed for embedding a Script language into your Application. 

- lightweight
- modular - all Objects i added to test the script and also the core function 
need to be registered - without it's the pure script language. 
- easy to bind a new function 
- easy to create new object types 
- full basic math support `+ - * /`, compare `== !=`, and/or `&& ||`  and bit shifting `<< >> | ^ &`
- Conditional logic with `if else elif` 
- Loops with `for while forRange forEach` 
- User function and methods: `fn [Namepace::]functionName [parameters]` 
- Variable Types are auto assigned `i = 5; f = 3.1; str="hello"` 

[Syntax Informations](./Syntax.md) 

[How to Bind function and objects](./Bindings.md) 

[Examples Folder](../examples) 

[Scripts Folder](../scripts) 

---

## Examples

### Hello World
```
print "Hello World"
```

### FizzBuzz  

```
for i 1 100
    if i % 15 == 0
        print "FizzBuzz"
    elif i % 3 == 0
        print "Fizz"
    elif i % 5 == 0
        print "Buzz"
    else
        print i;
    end
end
```

### Fibonacci  

```
-- calculate fibonacci n number
fn fib n                              # define a user function with parameter 'n'
    if n <= 1; return n;;             # 2 statements separated by ';' using ';;' as 'end' alias
    return (fib n - 1) + (fib n - 2)  # recursive function call. Brackets encapsulate the parameter scope
end

forRange i 14                         # forRange counts 'i' from 0 to 13, it does 14 loops
    print "Fib" i "=" (fib i)
end
```

### 99 Bottles of Beer

```
-- -----------------------------------------------------------------------------
-- DreiZehn examples/99Bottles.13
-- -----------------------------------------------------------------------------

-- we count from 99 down to 1
for i 99 1
    if i > 2
        -- print: simply write all parameters you want to print separated by a blank.
        --   Only if you want to print a function call you need to
        --   encapsulate the parameter scope by brackets like (myvec->at 0).
        --   Don't care about if it's float or int or string. DreiZehn does the
        --   convert for you.
        --   Note: If you add an object to print, it print the pointer number and type.
        --
        -- demonstrating '\' usage for multiple lines in one statement
        
        print i  "bottles of beer on the wall,"  i  "bottles of beer." \
                 "\nTake one down and pass it around,"  i - 1 \
                 "bottles of beer on the wall.\n";
    elif i == 2
        print "Two bottles of beer on the wall, Two bottles of beer." \
            "\nTake one down and pass it around, One bottle of beer on the wall.\n";
    else
        print "One bottle of beer on the wall, One bottles of beer." \
            "\nTake one down and pass it around, No bottles of beer on the wall.\n";
    end
end

print  "No more bottles of beer on the wall, no more bottles of beer." \
       "\nGo to the store and buy some more. 99 bottles of beer on the wall.\n";
```

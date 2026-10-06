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

forRange i 13                         # forRange counts 'i' from 0 to 13
    print "Fib" i "=" (fib i)
end
```

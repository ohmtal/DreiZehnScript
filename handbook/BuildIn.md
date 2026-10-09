# DreiZehn Script: Build in Functions

Functions are usually loaded with the import command like: `import Core`.

You may find my build in help usefull. This is placed in the Core Functions
(Verion 0.7 - this may change in the future) so you need to:

```
import Core
help
```
This will list the sub cathegories of help. `help::fn "core::"` for example. List
all functions which begins with "core::". 



## But there are also some build in functions:

### print

Print a line to the console. 

Example: `name = "tom"; print "hello" name 4711`

### printraw

Like `print` but without an line break so you can stack a line before you add 
an line break;

Example: `forRange i 10; printraw i "::"; end; printraw "\n"`

### error

Like `print` but tagged as error. 

### run

Run a script. This can also be used to include other scripts.

Example: `run "scripts/main.13"`


### modules

List the status of the available and loaded modules. 

Example: `modules`


### int::cast and float::cast

Cast a number or string to int or float. 

Examples: 
- `print (int::cast 3.14)` prints 3
- `print (int::cast "3.14")` prints 3
- `print (float::cast "3.14")` prints 3.14 
- `print (float::cast 3)` prints 3.00

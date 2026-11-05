# DreiZehn Cheat Sheet

| basic | (not complete) |
| --- | --- | 
| `print "hello world" 12 3.13`| no paren, space separated |
| `i=1; j=2`|semicolon only to separate multiple statement is on line else it's optional|
| `print (math::rand) 12`|encapsulate your nested function calls with paren|
| `if foo == 1; print "it's one"; end`|if, while, function need a `end`|
| `if foo == 1; print "it's one";;`| lazy alias for end is `;;` |
| `# this is a comment` | '#' comment. it can be used for one line| 
| `print "hello" # this is a comment` | also on the end| 
| math | |
| `f = (10 * 5 + 2 ) / 4 - 1`| basic math - it respect the order|
| `b = true && false` |  boolean and, or |
| `b++;b--` |  increment one and then decrement one |
| `b *= 2` |  multiply by 2 (`*=`, `+=`, `-=`, `/=`) |
| Loops |  |
| `forRange i 10; print i; end`|  for range loop from 0 to 9. | 
| `for i 10 1 2; print i; end`|  for loop from 10 down to 1 step 2| 
| `forEach item myVec; print item; end`|  iterate a object like a Vector for each item | 
| `i = 5; while i > 0; print i; i = i - 1; end`| while loop | 
| `i = 5; while i > 0; print i; if i == 2; continue;; if i == 1 break;; end`| continue and break | 
| Condition |  |
| `if true == false; print "Thats wrong!"; else print "OK"; end`  | if elif else |
| Function |  |
| `fn greeting name; print "Hello" name; end; greeting "there"`  | function greeting with parameter name  |
| `fn getPI; return 3.1415 end; print getPI`  | function getPI return 3.1415  |


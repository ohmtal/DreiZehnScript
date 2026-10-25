# DreiZehn Script: Bindings to C++

## Module Foo

Let's say we want to add a new Module "Foo"

We create a `RegisterFooFunctions` and add a function `foo::run`.

The RegisterFunction takes 2 Arguments:
- The script function name 
- The lamba which does the logic when the script function is called

The lamba takes 2 Arguments:
- std::vector<Value>& args - a list of the Values which are added when `foo::run` is called. 
Example: `foo::run 1 2 3` => three args all integer and are in arg[0]..arg[2].
- Value& ret - the return Value `print foo::run` will print the double value: 4711.0815

```
#include <span>
#include "core/FunctionMap.h"

void RegisterFooFunctions(Environment& env) {
    using namespace DreiZehn;
   
   FunctionMap::RegisterFunction("foo::run", [](std::span<Value> args, Value& ret) -> bool {
        printf("The Foo was here ...\n");
        ret = Value(4711.0815);
        return true;
    }, "Foo::run - return a magic number...");
}
```

DreiZehn uses a registry system so you have to add the RegisterFooFunctions to 
the ModuleRegistry when your Programm is loaded:

```
 FunctionMap::ModuleRegistry::Register("Foo", RegisterFooFunctions);
```

so in script you can load it with:
```
import Foo
```

if you don't need a import system you also can run `RegisterFooFunctions(env)` directly. 
The env comes from `Environment env;` see also main.cpp in the demo folder.


### Add a Object to Module::Foo

In your source before RegisterFooFunctions is called you can define a object like this:

```
using namespace DreiZehn;
const int TypeFooObject =  RegisterUserObjectType("FooObject");

struct FooObject : public ValueObject {
    FooObject() : ValueObject(TypeFooObject) { }
};

```

This is a very basic FooObject. In script you can use it like the `Object`. 

But you also need a Constructor for it. In you `void RegisterFooFunctions(Environment& env) {` add:

```
RegisterFunction("Foo::new", [&env](std::span<Value> args, Value& ret) -> bool {
    FooObject* obj = new FooObject();
    ret = Value(obj);
    return true;
}, "Create a new foo");
```

For an basic object that's it. From the ValueObject it got the dynamic FieldSystem and some methods.  

In Script you use it:

```
import Module::Foo
myFoo = Foo::new
myFoo.x = 10
myFoo.y = 20
myFoo->dump

```


For more informations and examples I suggest to look at 
**src/core/BaseObjects.cpp/h**, **src/functions/VectorFunctions.h**
or any other in the functions folder ;) 

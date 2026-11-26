# Parser 

My Parser growed by time adding stuff and i have the feeling it need some polish.
So i write this to make a Plan whats going on there.

## Overview

```
parse => parseStatement => parseComparison => parseMath => parsePrimary |
                                                    ^ -------------------
```


## ::parse

The Entry Point. It creates Blocks for the Body Statements
and start with calling: `parseStatement` When a line is done
or a "End" is reached it build a block out of it.

## ::parseStatement

ParseStatement handle the statements:
- `function`
- all loop  `for` and `while` types
- all `if` types
- `define`
- `import`
- `include`
- `global`
- EOL and Semicolon

If defaults to **parseComparison**

## ::parseComparison 

Starts with **parseMath** check it's a comparison and return a node.

## ::parseMath

This makes all the precedence magic ;) It start with **parsePrimary** to get a "left".

Then loop with current **precedence** and check minPrecedence. If passed it look for:

- Assign
- MathAssign
- Colon for Tenary (short if then)
- Inline Math (++/--)
- ~~Dot (field) and Arrow ?!~~ << **this is wrong ?**
- KeyValue for Struct constructor << **in Math?? - should it be in parsePrimary?**
- DotDot and DotAssign for String Concat
- Comma in line math experiment (ifdefed)

After this loop it raise the **precedence** and run parseMath recurive. 

Finally it handle the Math and Logical Operators.

## ::parsePrimary

- EOLToken
- Semicolon
- UnaryMinusExpression
- **lParan**
- LCurly
- Not
- Number (Literal)
- StringLiteral
- '&' for VariableReferenceExpression
- '*' for VariablePointerExpression
- Identifier => Call **parseIdentifier**




## ::parseExpression OBSOLETE 

It's only called by **'*' VariablePointerExpression** but does redunant code!
It's a old fragment before i changed to Precedence

- LParen
- Literals 
- Identifier 
- global 

I changed VariablePointerExpression and VariableReferenceExpression to parsePrimary
so this is obsolete !

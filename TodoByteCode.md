# DreiZehn Todo ByteCode

## Version 0.9

### NodeTypes:

```
debug::bc 'print "Hello word!"'
global b = 20; debug::bc 'print "Global " b ", " true;'
```

- [X] LiteralExpression
- [X] VariableExpression
- [x] ValueExpression
- [X] CallExpression cFunctions only at the moment  ...

---
 
```
 define foo = 47.11; global b = 20; debug::bc 'a= 10.1;print "Local:" a "Global " b ", CONST:" true "DEFINE" foo; '
```
- [ ] AssignStatement ( compileAssignment )
    - [X] VariableExpression
    - [ ] ArrayVariableExpression
    - [ ] MethodExpression
    - [ ] VariableReferenceExpression
    - [ ] VariablePointerExpression
    - [ ] ObjectFieldExpression

---
 
- [ ] CallExpression script functions 
- [ ] BlockStatement
- [ ] VariableReferenceExpression
- [ ] VariablePointerExpression
- [ ] ArrayVariableExpression
- [ ] MethodExpression
- [ ] BinaryExpression
- [ ] BinaryOpExpression
- [ ] BinaryInlineExpression
- [ ] IfStatement
- [ ] ElseMarkerNode
- [ ] ElIfStatement
- [ ] FunctionDefineStartNode
- [ ] EndNode
- [ ] ForStatement
- [ ] BreakStatement
- [ ] ContinueStatement
- [ ] ReturnStatement
- [ ] WhileStatement
- [ ] AssignOPStatement
- [ ] ObjectFieldExpression
- [ ] RangeStatement
- [ ] TernaryExpression
- [ ] ForEachStatement
- [ ] IncludeStatement
- [ ] KeyValueExpression


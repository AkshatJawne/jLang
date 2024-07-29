# jLang Compiler

jLang is a simple programming language designed as an introduction for me to compiler construction. It features an easy-to-understand syntax as follows:

```text
term = <input> | variable | literal
expression = term | term + term | ...
rel = term < term | ...
instr = variable = expression | <if> rel <then> instr | <goto> :label | <output> term | :label
```

Here is an example of a program in jLang:

```
m = input
j = 1
:start
output j
j = j + 2
if j < m then goto :start
if j < 15 then goto :end
output 42
:end
```

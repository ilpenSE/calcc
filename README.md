# calcc - Calculator in C

- It's a simple calculator written in C, supports +, -, *, /, % (mod), ^ (power) operators.
- Supports floating point numbers. (it's all doubles)
- Supports expressions within '()' (parenthesis) and they've the highest priority.
- It depends on libm (math library) additionally from libc.

## Quick Start

- Build the project in release mode (remove `r=1` to have debug one)
```bash
make r=1
```
- Change `CC` variable for different compilers, makefile is for linux environment.

- Lines start with `>>>` are input, `<<<` ones are output (result)
```bash
./main
>>> 1 + 2

<<< 3.00

>>> ...
```

- It'll report error nicely: (like GCC/Clang)
```bash
./main
>>> 1.2 + 1..4

ERROR: unexpected dot ('.') here
 |     1.2 + 1..4
 |             ^
total errors: 1

>>> .
```

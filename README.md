# sepl
A simple embedded programming language for c. sepl is a hobbyist project and is **not intended for production use**. The API and functionality may change without notice. Use at your own risk!

# Features
- No heap utilization
- No libc dependency
- ANSI c compatible

# Installation
Simply drag the single header file sepl.h into your project and add the following to your main.c file
```c
#define SEPL_IMPLEMENTATION
#include "sepl.h"
```

Or if you are using cmake add the sepl project dir as a cmake subdirectory and link the library
```cmake
add_subdirectory(path/to/sepl)
target_link_libraries(your_project sepl)
```

# Examples
Basic examples are provided in [examples/](examples/) directory

# Grammer
The grammer is provided in [Grammer.ebnf](Grammer.ebnf)

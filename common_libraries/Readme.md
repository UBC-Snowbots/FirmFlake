Common libraries

These have interfaces, and some hardware-agnostic libraries (In rare cases a hardware agnostic library is not fully agnostic - ie Atomic operations on cortex M0... but ehh)


Any interfaces should have corresponding implementations in the Platform folder

There are also unit tests for these libraries, but not all libraries have unit tests. We use Catch2. (This is why the common libraries folder has a CMakeLists and Makefile)

Each libarary/module should also have a readme, or comments explaining what its used for, how to use it, and any known issues.



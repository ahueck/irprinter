# irprinter &middot; ![License](https://img.shields.io/github/license/ahueck/irprinter)

*irprinter* is a command-line tool for exploring LLVM Intermediate Representation (IR) code. 
It allows users to print IR code for specific functions, which is particularly useful when dumping the entire translation unit would result in excessive output.


## Features
* Print LLVM IR code for a translation unit (C/C++) to the console.
* Modify and add compiler flags (e.g., replace -g with -O2) and regenerate the (modified) IR.
* Regex matching of (demangled) function names, with options to print:
  1. Function signatures only.
  2. Functions including their bodies.
* Dump the entire IR code of the translation unit.

## Usage
See [main.cpp](src/main.cpp) for all possible command-line arguments.

### Example of using *irprinter*
Assume *test.c* contains the code:

  ```c
  int foo () {
      return 2; 
  }
  int main() {
      int val;
      val = foo();    
      return 0;
  }
  ```
#### Using irprinter on test.c
In this example we
  1) load `test.c` with standard Clang flags,
  2) list all functions in `test.c`,
  3) print the body of main,
  4) optimize the code with `-O3`, and finally,
  5) print the body of main again.

```console
ahueck@sys:~/irprint/install$ ./bin/irprinter ../test.c --
ir-printer> l
Match 1 [foo]:
; Function Attrs: noinline nounwind optnone uwtable
define i32 @foo() #0 

Match 2 [main]:
; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 

ir-printer> p main
Match 1 [main]:
; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  %3 = call i32 @foo()
  store i32 %3, i32* %2, align 4
  ret i32 0
}

ir-printer> f -O3
Set flag to -O3. Re-generating module...
ir-printer> p main
Match 1 [main]:
; Function Attrs: norecurse nounwind readnone uwtable
define i32 @main() local_unnamed_addr #1 {
  ret i32 0
}

```

## How to build
###### Requirements
- CMake >= 3.20
- Clang/LLVM 12, 14, 18 (CMake needs to find the installation, see
  the [LLVM CMake documentation](https://llvm.org/docs/CMake.html) or the [CI workflow](.github/workflows/basic-ci.yml))
- C++17 compiler

###### Build steps
In the root project folder, execute the following commands (see also [CI workflow](.github/workflows/basic-ci.yml))

  ```
  cmake -B build -DCMAKE_INSTALL_PREFIX=*your path*
  cmake --build build --target install --parallel
  ```

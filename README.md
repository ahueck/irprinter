# irprinter

*irprinter* is a small commandline-based tool developed to explore LLVM IR code.
The tool can print IR code of only specific functions, when dumping of the whole translation unit would produce too much output.


## Main features
Print LLVM IR code of a translation unit (C/C++) to a console.
  - Ability to modify and add compiler flags (e.g., replace `-g` with `-O2`) and re-generate the (modified) IR
  - Regex matching of (demangled) function names, with the ability to print
      1) Function signatures only, and,
      2) Functions including body
  - Dump whole IR code of the TU

## Usage
See [main.cpp](src/main.cpp) for all possible commandline arguments.

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
- cmake >= 3.5
- LLVM 6.0 (cmake needs to find the installation, see the [LLVM cmake documentation](https://llvm.org/docs/CMake.html#id14))
- C++ compiler with support for the C++14 standard

###### Build steps
In the root project folder, execute the following commands

  ```
  mkdir build
  cmake .. -DCMAKE_INSTALL_PREFIX=*your path*
  cmake --build . --target install
  ```

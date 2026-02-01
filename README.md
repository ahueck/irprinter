# irprinter &middot; ![License](https://img.shields.io/github/license/ahueck/irprinter)

*irprinter* is a command-line tool for exploring LLVM Intermediate Representation (IR). 
It allows you to print IR for specific functions, which is especially useful when dumping an entire translation unit would produce excessive output.

## Features
* Print LLVM IR for a translation unit (C/C++) to the console.
* Modify or add compiler flags (e.g., replacing `-g` with `-O2`) and regenerate the IR.
* Match (demangled) function names using regular expressions, with options to print:
  1. Function signatures only.
  2. Full function bodies.
* Dump the entire IR of a translation unit.
* Print statements within a specific line number range (based on debug information).

## Usage
Refer to [main.cpp](src/main.cpp) for a full list of command-line arguments.

### Example
Assume `test.c` contains the following code:

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
In this example, we:
1. Load `test.c` with standard Clang and debug flags.
2. List all functions in `test.c`.
3. Print the body of `main`.
4. Print statements within the line range [6, 7].
5. Optimize the code with `-O3`.
6. Print the body of `main` again to see the result of the optimization.

```console
$~/irprint/install$ ./bin/irprinter ../test.c -- -g
ir-printer> l
Match 1 [foo()]:
; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z3foov() #0 !dbg !10 

Match 2 [main]:
; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #1 !dbg !10 

ir-printer> p main
Match 1 [main]:; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #1 !dbg !10 {
entry:
  %retval = alloca i32, align 4
  %val = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %val, !16, !DIExpression(), !17)
  %call = call noundef i32 @_Z3foov(), !dbg !18
  store i32 %call, ptr %val, align 4, !dbg !19
  ret i32 0, !dbg !20
}

ir-printer> 6 7
main:
  %call = call noundef i32 @_Z3foov(), !dbg !18
  store i32 %call, ptr %val, align 4, !dbg !19
  ret i32 0, !dbg !20

ir-printer> f -O3
Set flag to -O3. Re-generating module...
ir-printer> p main
Match 1 [main]:; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
entry:
  ret i32 0
}
```

## How to Build

### Requirements
- CMake >= 3.20
- Clang/LLVM 12, 14, or 18-21 (CMake must be able to find the installation; see the [LLVM CMake documentation](https://llvm.org/docs/CMake.html) or the [CI workflow](.github/workflows/basic-ci.yml))
- A C++17 compatible compiler

### Build Steps
From the root project folder, execute the following commands:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --target install --parallel
```
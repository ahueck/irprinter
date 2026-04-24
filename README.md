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
  * Printing backward dependencies for location queries is toggleable via `deps`.
  * Line-number prefixes are toggleable via `lines`.
    * Prefixes use `+` for dependency-discovered instructions.
    * A prefix like `+0 | ...` means the instruction has no debug line metadata in IR.

## Usage
Refer to [main.cpp](src/main.cpp) for a full list of command-line arguments.

### Example
Assume [test.c](test/codes/test.c) contains the following code:

```c
int foo() {
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

The regression test input for this flow lives at `test/codes/test.c`.

```console
$ ./bin/irprinter test/codes/test.c -- -g
ir-printer> l
Match 1 [foo()]:
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo() #0 !dbg !10 

Match 2 [main]:
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !10 

ir-printer> p main
Match 1 [main]:; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !10 {
entry:
  %retval = alloca i32, align 4
  %val = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %val, !16, !DIExpression(), !17)
  %call = call i32 @foo(), !dbg !18
  store i32 %call, ptr %val, align 4, !dbg !19
  ret i32 0, !dbg !20
}

ir-printer> 6 7
main:
  entry:
  +0 |   %val = alloca i32, align 4
   6 |   %call = call i32 @foo(), !dbg !18
   6 |   store i32 %call, ptr %val, align 4, !dbg !19
   7 |   ret i32 0, !dbg !20

ir-printer> f -O3
Set flag to -O3. Re-generating module...
ir-printer> p main
Match 1 [main]:; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
entry:
  ret i32 0
}
```

Note: this output was captured with LLVM 20.1.8. Exact IR attributes and mangling can vary between LLVM versions.

## How to Build

### Requirements
- CMake >= 3.20
- Clang/LLVM 12, 14, or 18-22 (CMake must be able to find the installation; see the [LLVM CMake documentation](https://llvm.org/docs/CMake.html) or the [CI workflow](.github/workflows/basic-ci.yml))
- A C++17 compatible compiler

### Build Steps
From the root project folder, execute the following commands:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --target install --parallel
```

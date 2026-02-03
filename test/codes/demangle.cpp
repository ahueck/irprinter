// RUN: echo "d _Z3foov" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: Demangled name: foo
void foo() {
}
int main() {
  return 0;
}
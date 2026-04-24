// RUN: echo "l foo" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: _Z3foov
// CHECK-NOT: main
void foo() {
}
int main() {
  return 0;
}
// RUN: echo "l" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: Match 1 [foo]
// CHECK: Match 2 [main]
void foo() {
}
int main() {
  return 0;
}
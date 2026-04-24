// RUN: echo "q" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK-NOT: ir-printer>
int main() {
  return 0;
}

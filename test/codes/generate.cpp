// RUN: echo "g" | %llvm-ir-printer %s -- | %filecheck %s
// Without "-f -g" no effect, but no explicit error either.
// CHECK-NOT: Error
int main() {
  return 0;
}

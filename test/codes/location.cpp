// RUN: echo "6 10" | %llvm-ir-printer %s -- -g | %filecheck %s

// CHECK: main:
// CHECK: store i32 1, ptr {{.*}}
// CHECK: store i32 2, ptr {{.*}}
int main() {
  int a = 1;
  int b = 2;
  return a + b;
}
// RUN: echo "6 10" | %llvm-ir-printer %s -- -g | %filecheck %s

// CHECK: main:
// CHECK: store i32 1, {{(ptr|i32\*)}} {{.*}}
// CHECK: store i32 2, {{(ptr|i32\*)}} {{.*}}
int main() {
  int a = 1;
  int b = 2;
  return a + b;
}

// RUN: echo -e "f -Wall -O3\np main" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: Set flags to -Wall -O3
// CHECK: define {{.*}} @main
int main() {
  return 0;
}

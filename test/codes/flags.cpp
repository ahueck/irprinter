// RUN: echo -e "f -Wall\nf -O3\np main" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: Set flag to -Wall
// CHECK: Set flag to -O3
// CHECK: define {{.*}} @main
int main() {
  return 0;
}

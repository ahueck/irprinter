// RUN: echo -e "f -O3\np main" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: Set flag to -O3. Re-generating module...
// CHECK: define {{.*}} @main{{.*}}
// CHECK-NOT: optnone
int main() {
  return 0;
}

// RUN: echo "dump" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: source_filename = "{{.*}}dump.cpp"
// CHECK: define {{.*}} @main
int main() {
  return 0;
}

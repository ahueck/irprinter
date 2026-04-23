// RUN: echo "l printf" | %llvm-ir-printer %s -- | %filecheck %s

// CHECK: Match 1 [printf]:
// CHECK-NEXT: declare {{.*}} @printf{{.*}}
extern "C" int printf(const char*, ...);
int main() {
  printf("test");
  return 0;
}

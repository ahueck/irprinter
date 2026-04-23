// RUN: printf "l\np main\n6 7\nf -O3\np main\n" | %llvm-ir-printer %s -- -g | %filecheck %s

#line 1 "test.c"
int foo() {
  return 2;
}
int main() {
  int val;
  val = foo();
  return 0;
}

// CHECK: Match 1 [foo]:
// CHECK: Match 2 [main]:
// CHECK: Match 1 [main]:
// CHECK: define {{.*}} @main{{.*}} !dbg
// CHECK: %call = call {{.*}} @foo(), !dbg
// CHECK: store i32 %call, ptr %val, align 4, !dbg
// CHECK: ret i32 0, !dbg
// CHECK: main:
// CHECK: {{[ +][[:space:]]*[0-9]+[[:space:]]*\|}} {{.*}}call {{.*}}@foo
// CHECK: {{[ +][[:space:]]*[0-9]+[[:space:]]*\|}} {{.*}}store i32 %call
// CHECK: {{[ +][[:space:]]*[0-9]+[[:space:]]*\|}} ret i32 0
// CHECK: Set flag to -O3. Re-generating module...
// CHECK: Match 1 [main]:
// CHECK: define {{.*}} @main{{.*}}
// CHECK-NOT: alloca
// CHECK: ret i32 0

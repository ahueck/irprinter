// RUN: echo "10 5" | %llvm-ir-printer %s -- | %filecheck %s --check-prefix=LOC-ORDER
// RUN: echo "5 invalid" | %llvm-ir-printer %s -- | %filecheck %s --check-prefix=LOC-PARSE
// RUN: echo "l [" | %llvm-ir-printer %s -- | %filecheck %s --check-prefix=REGEX-ERR

// LOC-ORDER: Error: end location (5) is less than start location (10)
// LOC-PARSE: Invalid end location: invalid
// REGEX-ERR: Invalid regex ({{.*}}): "["

int main() {
  return 0;
}

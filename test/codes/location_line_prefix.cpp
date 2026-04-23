// RUN: printf "6\nlines\n6\n" | %llvm-ir-printer %s -- -g | %filecheck %s --check-prefixes=ON,OFF

int main() {
  int a = 1;
  int b = a;
  return b;
}

// ON: main:
// ON: +0 |
// ON:  6 |
// ON:  6 | ret

// OFF-LABEL: Line prefixes disabled
// OFF: main:
// OFF-NOT: |

//
// This file tests metam.c, generate metam_test_1.c first with:
//    metam -o metam_test_1.c.gen.c metam_test_1.c
//
#include <stdio.h>

int main() {
#define MACROBEGIN printf("numbers: ");
#define MACROFUNC(X) printf("%d ", X);
#define MACROEND printf("\n")
#include "metam_test_1.c.gen.c"
  print(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
  print(1, 2, 3, 4, 5);
  return 0;
}

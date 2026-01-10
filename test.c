#include <stdio.h>

#define FUNCTION(A) printf(#A "\n")

#define _MACRO_1(arg1) FUNCTION(arg1)
#define _MACRO_2(arg1, arg2) FUNCTION(arg1), FUNCTION(arg2)

#define CALL_FUNCTION(...) _MACRO_COUNT(__VA_ARGS__, _MACRO_2, _MACRO_1)(__VA_ARGS__)
#define _MACRO_COUNT(arg1, arg2, macro, ...) macro

int main() {
    CALL_FUNCTION(1);
    CALL_FUNCTION(1, 2);
    return 0;
}

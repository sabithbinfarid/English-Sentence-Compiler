#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int square(int x) {
    int result = (x * x);
    return result;
}

int add(int a, int b) {
    return (a + b);
}

int main(void) {
    int s = square(7);
    int total = add(11, 9);
    printf("%d\n", s);
    printf("%d\n", total);
    return 0;
}

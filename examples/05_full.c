#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int factorial(int n) {
    int result = 1;
    int i = 1;
    while (i < n + 1) {
        result = result * i;
        i = i + 1;
    }
    return result;
}

int main(void) {
    int marks = 85;
    char* subject = "Compiler Design";
    printf("%s\n", "Subject:");
    printf("%s\n", subject);
    printf("%s\n", "Marks:");
    printf("%d\n", marks);
    if (marks > 49 && !(marks < 0)) {
        printf("%s\n", "Result: PASS");
    } else {
        printf("%s\n", "Result: FAIL");
    }
    int fact = factorial(5);
    printf("%s\n", "5! =");
    printf("%d\n", fact);
    double mix = 5.5;
    printf("%.4g\n", mix);
    return 0;
}

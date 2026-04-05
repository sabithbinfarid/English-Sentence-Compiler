#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int age = 21;
    double gpa = 3.85;
    char* name = "Rahim";
    int x = 42;
    double pi = 3.14159;
    char* city = "Dhaka";
    printf("%s\n", "Name");
    printf("%s\n", name);
    printf("%s\n", "Age");
    printf("%d\n", age);
    printf("%s\n", "GPA");
    printf("%.4g\n", gpa);
    printf("%s\n", city);
    return 0;
}

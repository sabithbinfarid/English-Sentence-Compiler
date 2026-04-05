#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int score = 72;
    if ((score > 79)) {
        printf("%s\n", "Grade A");
    } else {
        if ((score > 59)) {
            printf("%s\n", "Grade B");
        } else {
            printf("%s\n", "Grade C");
        }
    }
    return 0;
}

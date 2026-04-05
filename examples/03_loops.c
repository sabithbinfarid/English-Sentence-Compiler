#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    for (int __r1 = 0; __r1 < 5; ++__r1) {
        printf("%s\n", "Repeat loop tick");
    }
    int count = 5;
    while ((count > 0)) {
        printf("%d\n", count);
        count = (count - 1);
    }
    return 0;
}

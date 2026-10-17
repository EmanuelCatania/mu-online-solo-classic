// Records the wasm32 ABI that the 0.97k port depends on.
#include <stdio.h>

int main(void) {
    printf("sizeof(void*)=%u\n", (unsigned)sizeof(void*));
    printf("sizeof(int)=%u\n", (unsigned)sizeof(int));
    printf("sizeof(long)=%u\n", (unsigned)sizeof(long));
    printf("sizeof(long long)=%u\n", (unsigned)sizeof(long long));
    printf("sizeof(long double)=%u\n", (unsigned)sizeof(long double));
    printf("sizeof(float)=%u\n", (unsigned)sizeof(float));
    printf("sizeof(double)=%u\n", (unsigned)sizeof(double));
    return 0;
}

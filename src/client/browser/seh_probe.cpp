// Expected to fail. The client uses __try/__except in eight translation units.
int main(void) {
    int value = 0;
    __try {
        value = 1;
    } __except (1) {
        value = 2;
    }
    return value;
}

int add(int a, int b) {
    int result;
    result = a + b;
    return result;
}

int main(void) {
    int a;
    int b;
    a = 1;
    b = 2;
    if (a < b) {
        int a;
        a = b + 1;
        output(add(a, b));
    }
}

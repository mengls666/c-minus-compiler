
void print(int a[], int n) {
    int i;
    output(a);
    i = 0;
    while ( i < n ) {
        output(a[i]);
        i = i + 1;
    }
}
void main(void) {
    int x[10];
    int i;
    i = 0;
    while (i < 10) {
        x[i] = i;
        i = i + 1;
    }
    i = 0;
    while (i < 10) {
        output(x[i]);
        i = i + 1;
    }
    output(x);
    print(x, 10);
}

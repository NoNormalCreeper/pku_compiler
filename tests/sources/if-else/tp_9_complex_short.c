int main() {
    int a = 1;
    int b = 2, c= 3, d = 4;
    if ((a || b) && (c || d)) {
        return a;
    }
    if (0 && a && b && c) {
        d = 0;
    }
    return d;
    return c;
}
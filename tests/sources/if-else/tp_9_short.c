int main() {
    int a = 2;
    int b = 1;
    if (a || b) {
        return a;
    } else {
        b = 2;
    }
    
    int c = 3;
    int d = 4;
    if (c && d) {
        return c;
    }
    return d;
}
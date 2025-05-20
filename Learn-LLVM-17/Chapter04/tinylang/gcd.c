unsigned gcd(unsigned a, unsigned b) {
    if (b == 0)
    return a;
    while (b != 0) {
        unsigned t = a % b;
        a = b;
        b = t;
    }
    return a;
}
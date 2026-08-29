#include <iostream>
using namespace std;

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int x = 10;
    float pi = 3.14;
    bool activo = true;

    for (int i = 0; i < 5; i++) {
        cout << "factorial(" << i << ") = " << factorial(i) << endl;
    }

    if (x >= 10 && activo != false) {
        cout << "x es mayor o igual a 10" << endl;
    } else {
        cout << "x es menor que 10" << endl;
    }

    return 0;
}

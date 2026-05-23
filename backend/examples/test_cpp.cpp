#include <iostream>
using namespace std;

// Función que calcula el factorial
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int x = 10@;
    float pi = 3.14;
    bool activo = true;

    // Ciclo for
    for (int i = 0; i < 5; i++) {
        cout << "factorial(" << i << ") = " << factorial(i) << endl;
    }

    // Condicional
    if (x >= 10 && activo != false) {
        cout << "x es mayor o igual a 10" << endl;
    } else {
        cout << "x es menor que 10" << endl;
    }

    return 0;
}

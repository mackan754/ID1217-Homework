#include <stdio.h>
#include <math.h>

#define STEPS 1000

double step = 1.0 / STEPS;  

double f(double x) {
    return sqrt(1 - x * x);
}

double integrate(double x) {
    return (f(x) + f(x + step)) * step / 2; 
}

int main() {
    double sum = 0.0;
    double x = 0.0;
    
    for (int i = 0; i < STEPS; i++) {  
        x = i * step;  
        sum += integrate(x);
    }

    printf("Estimated Pi: %.15f\n", sum * 4);
    return 0;
}

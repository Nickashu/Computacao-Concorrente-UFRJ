#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int ehPrimo(long long int n) {
    int i;
    if (n<=1) return 0;
    if (n==2) return 1;
    if (n%2==0) return 0;
    for (i=3; i<sqrt(n)+1; i+=2)
        if(n%i==0) return 0;
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    long long N = atoll(argv[1]);
    long long total_primos = 0;
    for (long long i = 1; i <= N; i++) {
        if (ehPrimo(i)) total_primos++;
    }
    printf("Resultado Sequencial: Total de primos de 1 a %lld é %lld\n", N, total_primos);
    return 0;
}
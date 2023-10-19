//UNIVERSIDADE FEDERAL DO SUL E SULDESTE DO PARÁ
//ENGENHARIA DA COMPUTAÇÃO - TURMA 2019
//PROFESSOR: JOÃO VICTOR COSTA CARMONA
//DISCIPLINA: SISTEMAS OPERACIONAIS

#if defined(__linux__) || defined(__unix__)
    #define WARNING 1
#endif
#if defined(_WIN32) || defined(_WIN64)
    #define WARNING 0
#endif
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX 30

int main() {
    if(!WARNING) {
        system("cls");
        srand(time(NULL));
        init_state();
        animation();
        work_flow();
    } else {
        printf("ESTE PROGRAMA NAO FOI FEITO PARA SISTEMAS LINUX/UNIX :(");
    }
    return 0;
}

#include "functions.h"
#include "ascii.h"
#include "ascii2.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <string.h>
#define MAX 64

/*
    uma barra de carregamento (0%====100%) vai indicar o quão o processo foi realizado
    um valor vai ser inserido nas propriedades do processo para indicar
    o quão pesado ele é (int weight), ao chegar um novo processo na lista e com uma
    prioridade maior, a porcentagem é salva dentro de context, o processo com maior prioridade
    é executado e o que foi interrompido, quando chegar a sua vez, irá retornar de onde parou.
*/

struct position {
    int left;
    int top;
    int right;
    int bottom;
};

struct process {
    unsigned int pid;
    unsigned int priority;
    int context;
    int weight;
    Process* next;
};

struct waitlist {
    Process* first;
    int isChanged;
};

static void gotoxy(int x, int y) {
    HANDLE hStdout;
    COORD destCoord;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    destCoord.X = x;
    destCoord.Y = y;
    SetConsoleCursorPosition(hStdout, destCoord);
}

//recursos visuais
void box(int left, int top, int right, int bottom) {
    int i;
    gotoxy(left, top);printf("%c", 218);
    for(i=0;i<right-left-1;i++) printf("%c", 196);
    printf("%c", 191);
    for(i=1;i<bottom-top; i++) {
        gotoxy(left,top+i);printf("%c", 179);
        gotoxy(right, top+i);printf("%c", 179);
    }
    gotoxy(left, bottom);printf("%c", 192);
    for(i=0;i<right-left-1;i++ )printf("%c", 196);
    printf("%c", 217);
}

//esqueleto da interface
static void template(Position position) {
    box(position.left,position.top,position.right,position.bottom);
    
    //fila de espera
    box(position.left+4,position.top+15,position.right-65,position.bottom);
    char waitlist[] = "Processos em espera";
    int waitlistpos[2] = {((position.right-65-position.left+4+1)-strlen(waitlist))/2, position.bottom-position.top-14};
    gotoxy(waitlistpos[0]+position.left,waitlistpos[1]);
    printf("%s", waitlist);

    //fila de espera - cabeçalho
    box(position.left+4,position.top+15,position.right-65,position.bottom);
    //qtde de ansi char 1 234567                                       8 9 10 11
    char process[] = "\033[1;92mPID    Prioridade    Contexto    Peso\033[0m";
    int ansi_char = 11;
    int processpos[2] = {((position.right-65-position.left+4+1)-strlen(process+ansi_char))/2, position.bottom-position.top-12};
    gotoxy(processpos[0]+position.left,processpos[1]);
    printf("%s", process);
    
    //menu-logs
    box(position.left+53,position.top+15,position.right-3,position.bottom);
    char menu[] = "Menu/logs";
    int menupos[2] = {((position.left+53-position.right-3+1)-strlen(menu))/2, position.bottom-position.top-14};
    gotoxy(menupos[0]+position.right, menupos[1]);
    printf("%s", menu);

    char text1[] = "Processo em execucao";
    int text1pos[2] = {((position.right-position.left-2-strlen(text1))/2), position.bottom-position.top-25};
    gotoxy(text1pos[0]+position.left+2, text1pos[1]);
    printf("%s", text1);

    char text2[] = "PID      Prioridade      Contexto      Peso";
    int text2pos[2] = {((position.right-position.left-2-strlen(text2))/2), position.bottom-position.top-23};
    gotoxy(text2pos[0]+position.left+2, text2pos[1]);
    printf("%s", text2);

    char andamento[] = "Andamento";
    int andamentopos[2] = {((position.right-position.left-2-strlen(andamento))/2), position.bottom-position.top-19};
    gotoxy(andamentopos[0]+position.left+2, andamentopos[1]);
    printf("%s", andamento);
}

//limpa a fila de espera
void clear1(Position position) {
    int start=position.top+17, stop=position.bottom-1,
    wallLeft=position.left+5, wallRight=position.right-66;
    while(start<=stop) {
        while(wallLeft<=wallRight) {
            gotoxy(wallLeft, start);
            printf(" ");
            wallLeft++;
        }
        if(wallLeft>wallRight) {
            wallLeft = position.left+5;
        }
        start++;
    }
}

//limpa o menu-logs
void clear2(Position position) {
    int start=position.top+16, stop=position.bottom-1,
    wallLeft=position.left+54, wallRight=position.right-4;
    while(start<=stop) {
        while(wallLeft<=wallRight) {
            gotoxy(wallLeft, start);
            printf(" ");
            wallLeft++;
        }
        if(wallLeft>wallRight) {
            wallLeft = position.left+54;
        }
        start++;
    }
}

//desenha a lista de processos na tela
void draw_list(WaitList* list, Position position) {
    char** visible = list_print(list);
    if(visible!=NULL) {
        clear1(position);
        int start=position.top+17, stop=position.bottom-1;
        int i = 0, count = 0, padding = 0;
        while(visible[count]!=NULL){count++;}
        while(start<=stop && i<count-1) {
            int visiblepos[2] = {((position.right-65-position.left+4+1)-strlen(visible[i]))/2, position.bottom-position.top-11+i};
            gotoxy(visiblepos[0]+position.left-padding,visiblepos[1]);
            printf("%s", visible[i]);
            i++;
            start++;
        }
    }
    free(visible);
}

//desenha os menu e logs na tela
void draw_menu(Position position, int mode) {
    clear2(position);
    int items = 0, i = 0;
    if(mode == 0) {
        int padding = -10;
        char* menu[] = {"1 - Iniciar o processamento    ", "\033[31;1m0 - Sair\033[0m            ", NULL};
        while(menu[items]!=NULL) {items++;}
        while (i<items) {
            int menupos[2] = {((position.left+53-position.right-3+1)-strlen(menu[i]))/2, position.bottom-position.top-12+i};
            gotoxy(menupos[0]+position.right+padding, menupos[1]);
            printf("%s", menu[i]);
            i++;
        }
    } else if(mode==1){
        int padding = -10;
        char* menu[] = {"\033[93;1;6m1 - Inserir processo na fila\033[0m   ",
        "\033[31;1m0 - Sair\033[0m                         ", 
        "Para comecar, insira pelo menos 1 processo...",
        "Por favor, nao insira mais de 30 processos  ", NULL};
        while(menu[items]!=NULL) {items++;}
        while (i<items) {
            int menupos[2] = {((position.left+53-position.right-3+1)-strlen(menu[i])+13)/2, position.bottom-position.top-12+i};
            gotoxy(menupos[0]+position.right+padding, menupos[1]);
            printf("%s", menu[i]);
            i++;
        }
    } else if(mode== 2) {
        int padding = -14;
        char* menu[] = {"Insira uma quantidade: ", NULL};
        while(menu[items]!=NULL) {items++;}
        while (i<items) {
            int menupos[2] = {((position.left+53-position.right-3+1)-strlen(menu[i]))/2, position.bottom-position.top-12+i};
            gotoxy(menupos[0]+position.right+padding, menupos[1]);
            printf("%s", menu[i]);
            i++;
        }
    } else if(mode==3) {
        int padding = -6;
        char* menu[] = {"\033[96;93;1;6m1 - Interromper execucao\033[0m",
        "\033[31;1m0 - Sair\033[0m                    ", NULL};
        while(menu[items]!=NULL) {items++;}
        while (i<items) {
            int menupos[2] = {((position.left+53-position.right-3+1)-strlen(menu[i]))/2, position.bottom-position.top-12+i};
            gotoxy(menupos[0]+position.right+padding, menupos[1]);
            printf("%s", menu[i]);
            i++;
        }
    }
}

int work_flow() {
    //layout
    Position position;
    position.left = 4; position.top = 0;
    position.right = 115; position.bottom = 28;
    
    int pid = 1;
    WaitList* lista1 = create_list();

    int option = 1;
    int terminate = 0;
    do
    {
        switch (option)
        {
        case 0:
            {   
                hidecursor(1);
                system("cls");
                template(position);
                draw_list(lista1, position);
                draw_menu(position, 0);
                int c=0;
                while(c!= '1' && c != '0') {
                    do {} while(!kbhit());
                    c = getch();
                }
                if(c=='1'){
                    draw_menu(position, 3);
                    hidecursor(0);
                    terminate = trigger(lista1, &pid, position);
                    option = 1;
                }
                if(c=='0') terminate = 1;
            }
            break;
        case 1:
            {
                hidecursor(1);
                system("cls");
                template(position);
                draw_menu(position, option);
                int c=0;
                while(c!= '1' && c != '0') {
                    do {} while(!kbhit());
                    c = getch();
                }
                if(c=='1'){
                    draw_menu(position, 2);
                    hidecursor(0);
                    int nprocess = 0;
                    scanf("%i", &nprocess);
                    hidecursor(1);
                    if(nprocess<=0) {
                        option = 1;
                    } else {
                        for (int i = 0; i < nprocess; i++)
                        {
                            list_insert(lista1, create_process(&pid));
                        }
                        option = 0;
                    }
                } else if(c=='0') {
                    terminate=1;
                }
            }
            break;
        
        default:
            exit(1);
            break;
        }
    } while (terminate!=1);
    //fazer os free
    system("cls");
}

Process* create_process(int *i) {
    Process* p = (Process*) malloc(sizeof(Process));
    p->pid = *i;
    p->next = NULL;
    p->context = 0;
    p->weight = 50 + rand() / (RAND_MAX / (500 - 50 + 1) + 1);
    p->priority = (int)((double)rand() / ((double)RAND_MAX + 1) * 10);
    if(p->priority==0) p->priority = 1;
    *i = *i + 1;
    return p;
}

WaitList* create_list() {
    WaitList* l = (WaitList*) malloc(sizeof(WaitList));
    l->first = NULL;
    l->isChanged = 0;
    return l;
}

void list_insert(WaitList* list, Process* process) {
    if(list->first!=NULL) {
        Process* aux = list->first;
        while (aux!=NULL)
        {
            if(aux->next == NULL) {
                aux->next = process;
                break;
            }
            aux = aux->next;
        }
    } else {
        list->first = process;
    }
    list->isChanged = 1;
}

void list_remove(WaitList* list) {
    if(list!=NULL) {
        Process* aux = list->first;
        list->first = list->first->next;
        free(aux);
    }
}

char** list_print(WaitList* list) {
    Process* aux = list->first;
    if(aux!=NULL) {
        int i = 0;
        char** a = (char**) malloc(sizeof(char*)*MAX);
        while (i<MAX)
        {
            a[i] = (char*) malloc(sizeof(char)*MAX+1);
            i++;
        }

        char buffer[MAX] = "";
        i=0;
        while (aux!=NULL)
        {
            snprintf(buffer, sizeof(buffer), "%02i         %i           %02i%%       %04.1f",
            aux->pid, aux->priority,
            aux->context,
            (float) aux->weight/10);
            strcpy(a[i], buffer);
            a[i] = (char*) realloc(a[i], strlen(a[i])*sizeof(char)+1);
            i++;
            aux = aux->next;
        }
        a = (char**) realloc(a, (i+1)*sizeof(char*));
        a[i+1] = NULL;
        return a;
    } else {
        //está vazia
        return NULL;
    }
}

Process* scheduler(WaitList* list, Position position) {
    if(list->first==NULL) return NULL;
    //reorganiza a fila
    if(list->isChanged) {
        Process* current = list->first;
        Process* index;
        //algoritmo buble sort para a organização da fila
        while (current!=NULL) {
            index = current->next;
            while (index!=NULL) {
                if(current->priority > index->priority) {
                    int tmpPid = current->pid,
                    tmpPrt = current->priority,
                    tmpContext = current->context,
                    tmpWeight = current->weight;

                    current->pid = index->pid;
                    current->priority = index->priority;
                    current->context = index->context;
                    current->weight = index->weight;
                    
                    index->pid = tmpPid;
                    index->priority = tmpPrt;
                    index->context = tmpContext;
                    index->weight = tmpWeight;
                }
                index = index->next;
            }
            current = current->next;
        }
        list->isChanged = 0;
        draw_list(list, position);
        return list->first;
    } else {
        return  list->first;
    }
}

static void saveContext(Process* process, int context) {
    process->context = context;
}

static int recoveryContext(Process* process, int open, int close) {
    if(process->context>0) {
        return (((close-open)*process->context)/100)+open;
    } else {
        return open;
    }
}

static int run (WaitList* list, Process* process, Position position) {
    int close = 33, padding = 2;
    int open = 3, context = recoveryContext(process, open, close);
    int runPos[2] = {((position.right-position.left-padding-close-open)/2), position.bottom-position.top-20};
    
    //printa o processo em execução
    char in_execution[MAX];
    snprintf(in_execution, sizeof(in_execution), "%02i           %i              %02i%%        %04.1f",
        process->pid, process->priority,
        process->context,
        (float) process->weight/10);
    int text2pos[2] = {((position.right-position.left-2-strlen(in_execution))/2), position.bottom-position.top-22};
    gotoxy(text2pos[0]+position.left+2, text2pos[1]);
    printf("%s", in_execution);    

    //limpa a barra de loading
    while (open<close)
    {
        gotoxy(runPos[0]+position.left+padding+open, runPos[1]);
        printf(" ");
        open++;
    }

    open = 3;
    if(context > 3) {
        int i=0;
        char c[] = "=";
        //close-open
        char c2[30] = "";
        while (i < context-3){
            strcat(c2,c);
            i++;
        }
        gotoxy(runPos[0]+position.left+padding, runPos[1]);
        printf("0%%[%s", c2);
        open = context;
    } else {
        gotoxy(runPos[0]+position.left+padding, runPos[1]);
        printf("0%%[");
    }
    gotoxy(runPos[0]+position.left+close+padding, runPos[1]);
    printf("]100%%");
    int c = 0;
    while (open<close && c!='1' && c!='0') {
        if(kbhit()) {
            c = getch();
        }
        gotoxy(runPos[0]+position.left+padding+open, runPos[1]);
        printf("=");
        open++;
        Sleep(process->weight);
    }

    if(c=='0') {
        return 2;
    }
    if(c=='1') {
        //regra de 3
        context = (open-3)*100/(close-3);
        saveContext(process, context); 
        return 0;
    }
    
    //finalização da execução do processo
    Process* aux = process->next;
    list->first==process ? list_remove(list) : printf("o processo que voce quer deletar nao é o topo da fila");
    if(aux!=NULL && c != '0') {
        draw_list(list, position);
        run(list, aux, position);
    }
}

int dispatcher(WaitList* list, Process* process, Position position) {
    Process* current = list->first;
    return run(list, process, position);
}

static void middleware(WaitList* list, int* pid, Position position) {
    hidecursor(1);
    clear2(position);
    int nprocess = 0, padding = -13, items=0, i=0;
    char* menu[] = {"1 - Continuar execucao    ", "2 - Inserir mais processos", NULL};
    while(menu[items]!=NULL) {items++;}
    while (i<items) {
        int menupos[2] = {((position.left+53-position.right-3+1)-strlen(menu[i]))/2, position.bottom-position.top-12+i};
        gotoxy(menupos[0]+position.right+padding, menupos[1]);
        printf("%s", menu[i]);
        i++;
    }
    int c=0;
    while(c!= '1' && c != '2') {
        do {} while(!kbhit());
        c = getch();
    }
    if(c!='1') {
        clear2(position);
        i=0;items=0;
        char* menu2[] = {"Insira a quantidade: ", NULL};
        while(menu2[items]!=NULL) {items++;}
        while (i<items) {
            int menu2pos[2] = {((position.left+53-position.right-3+1)-strlen(menu2[i]))/2, position.bottom-position.top-12+i};
            gotoxy(menu2pos[0]+position.right+padding, menu2pos[1]);
            printf("%s", menu2[i]);
            i++;
        }
        hidecursor(0);
        scanf("%i", &nprocess);
        for(int i = 0; i < nprocess; i++) {
            list_insert(list, create_process(pid));
        }
        hidecursor(1);
        draw_list(list, position);
        draw_menu(position, 3);
    } else {
        draw_list(list, position);
        draw_menu(position, 3);
    }
}

int trigger(WaitList* list, int* pid, Position position) {
    hidecursor(1);
    while (list->first!=NULL)
    {
        int status = dispatcher(list, scheduler(list, position), position);
        if(status==0) {
            middleware(list, pid, position);
        } else if(status==2) {
            return 1;
        }
    }
    hidecursor(0);
    return 0;
}

int memory_cleaner(WaitList* list) {
    if(list->first==NULL) return 0;
    Process* aux = list->first;
    Process* aux2;
    while (aux!=NULL)
    {
        aux2 = aux->next;
        free(aux);
        aux = aux2;
    }
    list->first == NULL;
    return 0;
}

void init_state() {
    //muda o titulo da janela do console
    SetConsoleTitle("SO | SIMULADOR DE ESCALONAMENTO DE PROCESSOS - Beta 0.2.1");

    //altera a resolução do console
    SMALL_RECT windowSize = {0, 0, 119, 29};
    HANDLE wHnd;
    wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleWindowInfo(wHnd, 1, &windowSize);
    COORD bufferSize = {120, 1000};
    SetConsoleScreenBufferSize(wHnd, bufferSize);
    HWND consoleWindow = GetConsoleWindow();
    
    //impede o usuario de alterar o tamanho da tela
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
}

void print_animation(unsigned char* file)
{
    printf("\033[1;31;49m%s\033[0m",file);
}

int animation() {
    system("cls");
    hidecursor(1);
    int c = 0;
    int s = 0;
    
    while(c == 0)
    {
        do {
            gotoxy(0,0);
            if(s) {
                print_animation(ascii_txt);
                s=0;
            } else {
                print_animation(ascii2_txt);
                s=1;
            }
            Sleep(100);
        } while(!kbhit());
        c = getch();
    }
    system("cls");
    hidecursor(0);
}

void hidecursor(int hide) {
   HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   hide ?  (info.bVisible = FALSE) : (info.bVisible = TRUE);
   SetConsoleCursorInfo(consoleHandle, &info);
}

//não usada por enquanto
void setTimeout(unsigned int a) {
    int closeit = a; 
    int sec = 0;
    clock_t before = clock();

    system("cls");
    printf("Segundos: ");
    do {
        hidecursor(1);
        clock_t difference = clock() - before;
        sec = difference / CLOCKS_PER_SEC;
        gotoxy(10,0);
        printf("\033[1;32m%i\033[0m\n", sec);
    } while(sec < closeit);
    hidecursor(0);
}
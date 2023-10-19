//struct declaration
typedef struct process Process;
typedef struct waitlist WaitList;
typedef struct position Position;
//recursos visuais
int work_flow();

//main functions
void init_state();
Process* create_process(); //ok
WaitList* create_list(); //ok
void list_insert(WaitList* list, Process* process); //ok
void list_remove(WaitList* list); //ok
char** list_print(WaitList* list); //ok
Process* scheduler(WaitList* list, Position position); //ok
int dispatcher(WaitList* list, Process* process, Position position); //ok
int memory_cleaner(WaitList* list); //ok
int trigger(WaitList* list, int* pid, Position position); //ok


//etc
void hidecursor(int hide);
void setTimeout(unsigned int a);
int animation();
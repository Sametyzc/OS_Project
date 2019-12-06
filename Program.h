#ifndef HEADER_FILE
#define HEADER_FILE

#define READ_END 0
#define WRITE_END 1
#define System_Call 1

int NumberOfBackgroundProcess;
struct Commands {
   char * process[10];
   //Sistem cagrisimi degilmi belirtmek icin
   int type;
   //arka planda calisip calismayacagini anlamak icin
   int background;
};

void Background_Signal(int sig);
int Execvp(char * parameters[]);
int Execv(char * parameters[]);
int Pipe(struct Commands commands[], int number);
int Execute_Command(struct Commands command);
int outputRD(struct Commands commands[], int number, char file_name[]);
int Pipe_and_outputRd(struct Commands pipeCommands[], int pipe_number, char * file_name);
int inputRD(struct Commands commands[], int number, char * file_name);
int Recognizer(char * input);
void Prompt();

#endif

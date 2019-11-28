#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define READ_END 0
#define WRITE_END 1
#define System_Call 1

///Pipe icin coklu komutları kolay alabilmek icin struct yapisi

struct Commands{
  char* prosses[3];
  int type;
};

///Path degiskenini kullanan exec fonksiyonu

int Execvp(char* parameters[])
{
  if(execvp(parameters[0],parameters)<0){
    perror(parameters[0]);
    return -1;
  }
  return 0;
}

///Aynı dizindeki programları calistirmak icin exec fonksiyonu

int Execv(char* parameters[]){
  if(execv(parameters[0],parameters)<0)
  {
    perror(parameters[0]);
    return -1;
  }
  return 0;
}

//Gerekli komutlar commands degiskenine boyle yuklenmelidir
/*
struct Commands commands[4];
commands[0].prosses[0]="echo";
commands[0].prosses[1]="12";
commands[0].prosses[2]=NULL;
commands[0].type=1;

commands[1].prosses[0]="topla";
commands[1].prosses[1]=NULL;
commands[1].type=0;

commands[2].prosses[0]="topla";
commands[2].prosses[1]=NULL;
commands[2].type=0;

commands[3].prosses[0]="topla";
commands[3].prosses[1]=NULL;
commands[3].type=0;

if(Pipe(commands,4)<0)
{
  printf("Pipe olusurken hata");
}
*/
int Pipe(struct Commands commands[],int number){

  int fd[2];
  pid_t pid;
  int fdd = READ_END;

  for(int i=0;i<number;i++){
    //fd üzerinde pipe oluşturuldu
    pipe(fd);
    //Cocuk olusurken hata var mı diye kontrol edildi
    if ((pid = fork()) == -1) {
      perror("fork");
      return -1;
    }
    //Cocuk icinde yapılacak şeyler buraya yazılır
    else if (pid == 0) {
      //Ebeveyn ile habarleşmek için
      dup2(fdd, STDIN_FILENO);
      //Eger son komut degilse bilgisini bir sonraki komut için yazma ucuna yazar
      if (i != (number-1)) {
        dup2(fd[WRITE_END], STDOUT_FILENO);
      }
      //Kullanılmayan pipe lar kapatılır
      close(fd[READ_END]);
      //Eger girilmiş olan komut sistem komutu ise Execvp calistirilir
      if(commands[i].type==System_Call)
      {
        if(Execvp(commands[i].prosses)<0)
        return -1;
      }
      //Eger sistem komutu degil ise execv calisir
      else
      {
        if(Execv(commands[i].prosses)<0)
        {
          return -1;
        }
      }
    }
    //Ebeveyn cocukları bekler
    else {
      wait(NULL);
      //Kullanılmayan uc kapatılır
      close(fd[WRITE_END]);
      //Cocugun yazma ucu Ebeveynin okuma ucuna esitlenir
      fdd = fd[READ_END];
    }
  }
  return 0;
}


int Recognizer(char * input) {
  //printf("%s",input);
  char * pch;
  char programOutput[CHAR_MAX];
  //dizinin işaretlere göre ayrılması ve pch pointerın ilk bloğu göstermes(strtok belirlenen işaretleri \0 a dönüştürür)
  pch = strtok(input, " ;|,-");
  //pch pointerın diğer blokları göstermesi
  while (pch != NULL) {
    int i = 0;
    printf("genel  :%s\n", pch);
    //blokların ayrı ayrı incelenmesi
    while ( * (pch) != '\0') {
      printf("dizi  :%c\n", * pch);
      //blokların bir diziye aktarılması
      programOutput[i] = * (pch);
      pch++, i++;
      printf("1");
      //bloktan sonra < geliyor ise  bunun öncesindeki blok alınır.
      if ( * (pch + 1) == '<') {
        programOutput[i] = * (pch);
        printf("----cıktı :%s\n", programOutput);

      }
    }
    //pointerın  sonraki bloğu göstermesini sağlar
    pch = strtok(NULL, " ,.-");

  }

}
int Prompt() {
  char hostname[HOST_NAME_MAX];
  char username[LOGIN_NAME_MAX];
  char cwd[1024];

  //getcwd şu anki dosyanın konumunu getirir.
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    //hostname ve username
    getlogin_r(username, LOGIN_NAME_MAX);
    gethostname(hostname, HOST_NAME_MAX);

    printf("\033[1;36m%s@%s:\033[1;35m~%s\033[0;33m>", username, hostname, cwd);
    //Bufferda tutulan output verileri temizlendi.   
    fflush(stdout);

  }
  return 0;
}

int main() {
  char input[CHAR_MAX];
  while (1) {
    Prompt();
    fgets(input, sizeof input, stdin);

    Recognizer(input);
  }

  return 0;
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>


/* Pipe denemek için örnek 2 kod
  char* komutlar[]={"ls","less"};
  char* girdiler[]={"-1",NULL};
  if(Pipe(komutlar,girdiler)<0)
  {
    printf("pipe olusurken hata\n");
  }
  return 0;

*/
//Pipe için girdi ve çıktı uçlarını belirttik
#define READ_END 0
#define WRITE_END 1
//Commands komutların bulunduğu bir dizi
//Arguments commands ta sıfırıncı indikste hangi komut var ise arguments[0] da da onun parametresi olmalı
int Pipe(char* commands[],char* arguments[]){
  //Hiç komut girilmediyse hata veriyor
  if(commands[0]==NULL)
  {
    printf("Girilen komutlar yeterli degil!");
    return -1;
  }
  //Fork yapmak için child id si
  pid_t child_pid,child_pid2;
  //pipe in girdi ve çıktı ucunu belirten dizi
  int fd[2];
  // dizimiz üzerinde pipe oluşturuldu
  pipe(fd);
  //Cocuk olusturmak için fork yapıldı
  child_pid = fork();
  //execvp fonksiyonuna parametre vermek için listeler
  char* values1[] = {commands[0],arguments[0],NULL};
  char* values2[] = {commands[1],arguments[1],NULL};
  if(child_pid<0)
  {
    perror("Cocuk olusurken hata!");
    return -1;
  }
  else if(child_pid==0)
  {
    //dup2 fonksiyonu kullanılarak printf fonksiyonlarının çıktısı pipe yazıldı;
    //dup2 yaptıktan sonra tüm printfler ilk parametresine verilen yere yazılır burada pipe in girdi ucuna yazdırıldı
    dup2(fd[WRITE_END], STDOUT_FILENO);
    //Bunları niye kapatıyoruz bilmiyorum
    close(fd[READ_END]);
    close(fd[WRITE_END]);
    //execvp fonksiyonuna hangi komutun çalışacağı ve parame
    if(execvp(commands[0],values1)<0){
      perror(commands[0]);
      return -1;
    }
  }
  else
  {
    child_pid2=fork();
    if(child_pid2<0)
    {
      perror("Cocuk olusurken hata!");
      return -1;
    }
    else if(child_pid2==0)
    {
      dup2(fd[READ_END], STDIN_FILENO);
      close(fd[WRITE_END]);
      close(fd[READ_END]);
      if(execvp(commands[1],values2)<0){
        perror(commands[1]);
        return -1;
      }
    }
    else{
      wait(&child_pid2);
    }
    wait(&child_pid);
  }
  return 1;
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

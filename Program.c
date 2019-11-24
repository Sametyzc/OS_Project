#include <stdio.h>

#include <string.h>

#include <unistd.h>

#include <limits.h>

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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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

int Recognizer(char* input)
{
  char *pch, *pch2, *pch3;
  char *programOutput[CHAR_MAX], *pipeDirection[CHAR_MAX], *temp[CHAR_MAX], *process[CHAR_MAX],
  *parameter[CHAR_MAX];
  ;
  int a = 0, pipebool[CHAR_MAX] = {}, directionbool[CHAR_MAX] = {};
  pch = strtok(input, ";");
  while (pch != NULL)
  {
    pch2 = pch;

    printf("genel  :%s\n", pch);
    programOutput[a] = pch;
    while (*(pch) != '\0')
    {
      if (*pch == '|')
      {
        pipebool[a] = 1;
      }
      if (*pch == '<')
      {
        directionbool[a] = 1;
      }
      if (*pch == '>')
      {
        directionbool[a] = 2;
      }


      pch++;
    }


    pch = strtok(NULL, ";");
    a++;
  }

  for (int i = 0; i < a; i++)
  {
    //////////////////////////////////////////////////////////////////////////////////////////
    if (directionbool[i] == 1 && pipebool[i] == 1)
    {
      pch2 = strtok(programOutput[i], "<");
      int j = 0;
      while (pch2 != NULL)
      {
        pipeDirection[j] = pch2;
        printf("genel 1-1  :%s\n", pch2);

        pch2 = strtok(NULL, "<");
        j++;
      }
      // pipeDirection[i-1]=FILE

      int k = 0;
      pch2 = strtok(pipeDirection[0], "|");
      while (pch2 != NULL)
      {
        temp[k] = pch2;
        printf("genel 1-1-a  :%s\n", pch2);


        pch2 = strtok(NULL, "|");
        k++;
      }
      for (int l = 0; l < k; l++)
      {
        pch2 = strtok(temp[l], " ");
        int m = 0, counter = 0;
        while (pch2 != NULL)
        {
          if (m == 0)
          {
            if (*(pch2) == '.' && *(pch2 + 1) == '/')
            {
              process[counter] = pch2 + 2;
              printf("işlem %s        ", pch2 + 2);
            }
            else
            {
              process[counter] = pch2;
              printf("işlem %s        ", pch2);
            }
          }
          else if (m == 1)
          {
            parameter[counter] = pch2;
            printf("parametresi %s\n", pch2);
          }


          pch2 = strtok(NULL, " ");
          m++;
        }
        if (m == 1)
        {
          parameter[counter] = NULL;
          printf("parametresi %s\n", parameter[counter]);
        }
        counter++;
      }
      printf("Dosyası %s", pipeDirection[j - 1]);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    if (directionbool[i] == 2 && pipebool[i] == 1)
    {
      pch2 = strtok(programOutput[i], ">");
      int j = 0;
      while (pch2 != NULL)
      {
        pipeDirection[j] = pch2;
        printf("genel 2-1  :%s\n", pch2);

        pch2 = strtok(NULL, ">");
        j++;
      }
      // pipeDirection[i-1]=FILE

      int k = 0;
      pch2 = strtok(pipeDirection[0], "|");
      while (pch2 != NULL)
      {
        temp[k] = pch2;
        printf("genel 2-1-a  :%s\n", pch2);


        pch2 = strtok(NULL, "|");
        k++;
      }
      for (int l = 0; l < k; l++)
      {
        pch2 = strtok(temp[l], " ");
        int m = 0, counter = 0;
        while (pch2 != NULL)
        {
          if (m == 0)
          {
            if (*(pch2) == '.' && *(pch2 + 1) == '/')
            {
              process[counter] = pch2 + 2;
              printf("işlem %s        ", pch2 + 2);
            }
            else
            {
              process[counter] = pch2;
              printf("işlem %s        ", pch2);
            }
          }
          else if (m == 1)
          {
            parameter[counter] = pch2;
            printf("parametresi %s\n", pch2);
          }


          pch2 = strtok(NULL, " ");
          m++;
        }
        if (m == 1)
        {
          parameter[counter] = NULL;
          printf("parametresi %s\n", parameter[counter]);
        }
        counter++;
      }
      printf("Dosyası %s", pipeDirection[j - 1]);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    if (directionbool[i] == 1 && pipebool[i] == 0)
    {
      pch2 = strtok(programOutput[i], "<");
      int j = 0;
      while (pch2 != NULL)
      {
        pipeDirection[j] = pch2;
        printf("genel 1-0  :%s\n", pch2);

        pch2 = strtok(NULL, "<");
        j++;
      }
      for (int l = 0; l < j - 1; l++)
      {
        pch2 = strtok(pipeDirection[l], " ");
        int m = 0, counter = 0;
        while (pch2 != NULL)
        {
          if (m == 0)
          {
            if (*(pch2) == '.' && *(pch2 + 1) == '/')
            {
              process[counter] = pch2 + 2;
              printf("işlem %s        ", pch2 + 2);
            }
            else
            {
              process[counter] = pch2;
              printf("işlem %s        ", pch2);
            }
          }
          else if (m == 1)
          {
            parameter[counter] = pch2;
            printf("parametresi %s\n", pch2);
          }


          pch2 = strtok(NULL, " ");
          m++;
        }
        if (m == 1)
        {
          parameter[counter] = NULL;
          printf("parametresi %s\n", parameter[counter]);
        }
        counter++;
      }
      printf("Dosyası %s", pipeDirection[j - 1]);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    if (directionbool[i] == 2 && pipebool[i] == 0)
    {
      pch2 = strtok(programOutput[i], ">");
      int j = 0;
      while (pch2 != NULL)
      {
        pipeDirection[j] = pch2;
        printf("genel 1-0  :%s\n", pch2);

        pch2 = strtok(NULL, ">");
        j++;
      }
      for (int l = 0; l < j - 1; l++)
      {
        pch2 = strtok(pipeDirection[l], " ");
        int m = 0, counter = 0;
        while (pch2 != NULL)
        {
          if (m == 0)
          {
            if (*(pch2) == '.' && *(pch2 + 1) == '/')
            {
              process[counter] = pch2 + 2;
              printf("işlem %s        ", pch2 + 2);
            }
            else
            {
              process[counter] = pch2;
              printf("işlem %s        ", pch2);
            }
          }
          else if (m == 1)
          {
            parameter[counter] = pch2;
            printf("parametresi %s\n", pch2);
          }


          pch2 = strtok(NULL, " ");
          m++;
        }
        if (m == 1)
        {
          parameter[counter] = NULL;
          printf("parametresi %s\n", parameter[counter]);
        }
        counter++;
      }
      printf("Dosyası %s", pipeDirection[j - 1]);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    if (directionbool[i] == 0 && pipebool[i] == 0)
    {
      pch2 = strtok(programOutput[i], " ");
      int m = 0, counter = 0;
      while (pch2 != NULL)
      {
        if (m == 0)
        {
          if (*(pch2) == '.' && *(pch2 + 1) == '/')
          {
            process[counter] = pch2 + 2;
            printf("işlem %s        ", pch2 + 2);
          }
          else
          {
            process[counter] = pch2;
            printf("işlem %s        ", pch2);
          }
        }
        else if (m == 1)
        {
          parameter[counter] = pch2;
          printf("parametresi %s\n", pch2);
        }


        pch2 = strtok(NULL, " ");
        m++;
      }
      if (m == 1)
      {
        parameter[counter] = NULL;
        printf("parametresi %s\n", parameter[counter]);
      }
      counter++;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    if (directionbool[i] == 0 && pipebool[i] == 1)
    {
      int k = 0;
      pch2 = strtok(programOutput[i], "|");
      while (pch2 != NULL)
      {
        temp[k] = pch2;
        printf("genel 1-1-a  :%s\n", pch2);


        pch2 = strtok(NULL, "|");
        k++;
      }

      struct Commands commands[k];
      for (int l = 0; l < k; l++)
      {
        pch2 = strtok(temp[l], " ");
        int m = 0, counter = 0,counter2=0;
        while (pch2 != NULL)
        {
          if (m == 0)
          {
            if (*(pch2) == '.' && *(pch2 + 1) == '/')
            {
              printf("%icounter2:",counter2);
              commands[l].type=0;
              commands[l].prosses[0]=pch2+2;
              //  process[counter] = pch2 + 2;
              printf("işlem %s        ", pch2 + 2);
            }
            else
            {
              commands[l].type=1;
              commands[l].prosses[0]=pch2;
              //process[counter] = pch2;
              printf("işlem %s        ", pch2);
            }
          }
          else if (m == 1)
          {
            commands[l].prosses[1]=pch2;
            commands[l].prosses[2]=NULL;
            //parameter[counter] = pch2;
            printf("parametresi %s\n", pch2);
          }
          pch2 = strtok(NULL, " ");
          m++;
        }
        if (m == 1)
        {
          commands[l].prosses[1]=NULL;
          parameter[counter] = NULL;
          printf("parametresi %s\n", parameter[counter]);
        }
        counter++;

      }
      if(Pipe(commands,k)<0)
      {
        printf("pipe olusurken hata!\n");
      }
      printf("işlem sayisi :%i\n", k);
    }
  }
}
/*pch=strtok(limit[counter],"|");
while(pch != NULL){ printf("PIPE  :%s\n", pch);
pch = strtok(NULL, "|");}*/

int Prompt()
{
  char hostname[HOST_NAME_MAX];
  char username[LOGIN_NAME_MAX];
  char cwd[1024];

  // getcwd şu anki dosyanın konumunu getirir.x
  if (getcwd(cwd, sizeof(cwd)) != NULL)
  {
    // hostname ve username
    getlogin_r(username, LOGIN_NAME_MAX);
    gethostname(hostname, HOST_NAME_MAX);

    printf("\033[1;36m%s@%s:\033[1;35m~%s\033[0;33m>", username, hostname, cwd);
    // Bufferda tutulan output verileri temizlendi.
    fflush(stdout);
  }
  return 0;
}

int main()
{
  char input[1024];

  while(1)
  {
    Prompt();
    fgets(input, sizeof input, stdin);
    for(int i=0;i<1024;i++)
    {
      if(input[i]=='\n')
      {
        input[i]='\0';
      }
    }
    Recognizer(input);
  }

  /*
  struct Commands commands[4];
  commands[0].prosses[0]="echo";
  commands[0].prosses[1]="12";
  commands[0].prosses[2]=NULL;
  commands[0].type=1;

  commands[1].prosses[0]="topla";
  commands[1].prosses[1]=NULL;
  commands[1].type=0;

  commands[2].prosses[0]="";
  commands[2].prosses[1]=NULL;
  commands[2].type=0;

  commands[3].prosses[0]="topla";
  commands[3].prosses[1]=NULL;
  commands[3].type=0;
  if(Pipe(commands,4)<0)
  {
  printf("pipede hata\n");
}*/
return 0;
}

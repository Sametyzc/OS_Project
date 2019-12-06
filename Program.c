#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Program.h"

///Arka plan calismasi yapilirken
void Background_Signal(int sig) {
   int wstatus, child_pid;
   ///Eger arkaplan da calisan bir process var ise onu beklemek icin
   child_pid = waitpid(-1, & wstatus, WNOHANG);
   //Eger waitpid den gelen deger sifirdan buyuk ise bu arka planda calisan bir process dir
   if (child_pid > 0) {
      //Eger normal olarak calistirilirsa
      if (WIFEXITED(wstatus)) {
         //Ekrana donus degerini ve idsini yazar
         fprintf(stderr, "[%d] retval:%d\n", child_pid, WEXITSTATUS(wstatus));
         NumberOfBackgroundProcess--;
      }
   }
}
/// Path degiskenini kullanan exec fonksiyonu

int Execvp(char * parameters[]) {
   if (execvp(parameters[0], parameters) < 0) {
      perror(parameters[0]);
      return -1;
   }
   return 0;
}

/// Ayni dizindeki programlari calistirmak icin exec fonksiyonu

int Execv(char * parameters[]) {
   if (execv(parameters[0], parameters) < 0) {
      perror(parameters[0]);
      return -1;
   }
   return 0;
}

int Pipe(struct Commands commands[], int number) {
   int fd[2];
   pid_t pid;
   int fdd = READ_END;
   for (int i = 0; i < number; i++) {
      // fd uzerinde pipe olusturuldu
      pipe(fd);
      // Cocuk olusurken hata var mi diye kontrol edildi
      if ((pid = fork()) == -1) {
         perror("fork");
         return -1;
      }
      // Cocuk icinde yapilacak seyler buraya yazdirilir
      else if (pid == 0) {
         // Ebeveyn ile habarlesmek icin
         dup2(fdd, STDIN_FILENO);
         // Eger son komut degilse bilgisini bir sonraki komut un'in yazma ucuna yazar
         if (i != (number - 1)) {
            dup2(fd[WRITE_END], STDOUT_FILENO);
         }
         // Kullanilmayan pipe lar kapatilir
         close(fd[READ_END]);
         // Eger girilmis olan komut sistem komutu ise Execvp calistirilir
         if (commands[i].type == System_Call) {
            if (Execvp(commands[i].process) < 0)
               return -1;
         } else {
            if (Execv(commands[i].process) < 0) {
               return -1;
            }
         }
      }
      // Ebeveyn cocuklari bekler
      else {
         wait(NULL);
         // Kullanilmayan uc kapatilir
         close(fd[WRITE_END]);
         // Cocugun yazma ucu Ebeveynin okuma ucuna esitlenir
         fdd = fd[READ_END];
      }
   }
   return 0;
}

int Execute_Command(struct Commands command) {
   pid_t pid;
   if (command.background == 1) {
      struct sigaction act;
      act.sa_handler = Background_Signal;
      sigemptyset( & act.sa_mask);
      act.sa_flags = SA_NOCLDSTOP;
      if (sigaction(SIGCHLD, & act, NULL) < 0) {
         perror("sigaction");
         return -1;
      }
   }
   // Cocuk olusurken hata var mi diye kontrol edildi
   if ((pid = fork()) == -1) {
      perror("fork");
      return -1;
   }
   // Cocuk icinde yapilacak seyler buraya yazilir
   else if (pid == 0) {
      if (command.type == System_Call) {
         if (Execvp(command.process) < 0)
            return -1;
      } else {
         if (Execv(command.process) < 0)
            return -1;
      }
   }
   // Ebeveyn cocuklari bekler
   else {
      if (command.background == 0) {
         wait(NULL);
      } else {
         NumberOfBackgroundProcess++;
      }
   }
   return 0;
}
int outputRD(struct Commands commands[], int number, char file_name[]) {
   pid_t pid;
   int fileDescriptor;
   int background = 0;
   for (int i = 0; i < number; i++) {
      if (commands[i].background == 1) {
         background = 1;
         break;
      }
   }
   if (background == 1) {
      struct sigaction act;
      act.sa_handler = Background_Signal;
      sigemptyset( & act.sa_mask);
      act.sa_flags = SA_NOCLDSTOP;
      if (sigaction(SIGCHLD, & act, NULL) < 0) {
         perror("sigaction");
         return -1;
      }
   }
   if ((pid = fork()) == -1) {
      perror("fork:");
      return -1;
   }
   if (pid == 0) {
      fileDescriptor = open(file_name, O_CREAT | O_APPEND | O_WRONLY, 0600);
      dup2(fileDescriptor, STDOUT_FILENO);
      close(fileDescriptor);

      if (commands[0].type == 1) {
         if (Execvp(commands[0].process) < 0) {
            kill(getpid(), SIGTERM);
            return -1;
         }
      } else {
         if (Execv(commands[0].process) < 0) {
            kill(getpid(), SIGTERM);
            return -1;
         }
      }

   } else {
      if (background == 0) {
         wait(NULL);
      } else {
         NumberOfBackgroundProcess++;
      }
   }
   return 0;
}
int Pipe_and_outputRd(struct Commands pipeCommands[], int pipe_number, char * file_name) {
   pid_t pid;
   int fileDescriptor;
   int background = 0;
   for (int i = 0; i < pipe_number; i++) {
      if (pipeCommands[i].background == 1) {
         pipeCommands[i].background = 0;
         if (background == 0) {
            background = 1;
         }
      }
   }
   if (background == 1) {
      struct sigaction act;
      act.sa_handler = Background_Signal;
      sigemptyset( & act.sa_mask);
      act.sa_flags = SA_NOCLDSTOP;
      if (sigaction(SIGCHLD, & act, NULL) < 0) {
         perror("sigaction");
         return -1;
      }
   }
   if ((pid = fork()) == -1) {
      perror("fork:");
      return -1;
   }
   if (pid == 0) {
      fileDescriptor = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0600);
      if (fileDescriptor < 0) {
         perror("file");
         return -1;
      }
      if (dup2(fileDescriptor, STDOUT_FILENO) < 0) {
         perror("dup2");
         return -1;
      }
      if (close(fileDescriptor) < 0) {
         return -1;
      }

      if (Pipe(pipeCommands, pipe_number) < 0) {
         kill(getpid(), SIGTERM);
         return -1;
      }
   } else {
      if (background == 0) {
         wait(NULL);
      } else {
         NumberOfBackgroundProcess++;
      }
   }
   return 0;
}
int inputRD(struct Commands commands[], int number, char * file_name) {
   pid_t pid;
   int fileDescriptor;
   int background = 0;
   if (!(access(file_name, F_OK) != -1)) {
      fprintf(stderr, "access: %s adinda bir dosya bulunamadi\n", file_name);
      return -1;
   }
   for (int i = 0; i < number; i++) {
      if (commands[i].background == 1) {
         background = 1;
         break;
      }
   }
   if (background == 1) {
      struct sigaction act;
      act.sa_handler = Background_Signal;
      sigemptyset( & act.sa_mask);
      act.sa_flags = SA_NOCLDSTOP;
      if (sigaction(SIGCHLD, & act, NULL) < 0) {
         perror("sigaction");
         return -1;
      }
   }
   if ((pid = fork()) == -1) {
      perror("fork:");
      return -1;
   }
   if (pid == 0) {
      fileDescriptor = open(file_name, O_RDONLY, 0600);
      dup2(fileDescriptor, STDIN_FILENO);
      close(fileDescriptor);

      if (commands[0].type == 1) {
         if (Execvp(commands[0].process) < 0) {
            kill(getpid(), SIGTERM);
            return -1;
         }
      } else {
         if (Execv(commands[0].process) < 0) {
            kill(getpid(), SIGTERM);
            return -1;
         }
      }

   } else {
      if (background == 0) {
         wait(NULL);
      } else {
         NumberOfBackgroundProcess++;
      }
   }
   return 0;
}
//////KULLANICIDAN ALINAN GIRDININ AYIRILIP TANINMASI/////
int Recognizer(char * input) {
   char * pch, * pch2, * pch3;
   char * programOutput[CHAR_MAX], * pipeDirection[CHAR_MAX], * temp[CHAR_MAX], * process[CHAR_MAX],
      * parameter[CHAR_MAX];
   int a = 0, pipebool[CHAR_MAX] = {}, directionbool[CHAR_MAX] = {};
   // Kullanicidan alinan girdinin ';' isaretine gore bloklara ayrilmasi
   pch = strtok(input, ";");
   while (pch != NULL) {
      pch2 = pch;

      programOutput[a] = pch;
      // Bloklarina ayrilan girdinin pipe ve yonlendirme iC'erme durumunun kontrolu
      while ( * (pch) != '\0') {
         if ( * pch == '|') {
            pipebool[a] = 1;
         }
         if ( * pch == '<') {
            directionbool[a] = 1;
         }
         if ( * pch == '>') {
            directionbool[a] = 2;
         }

         pch++;
      }

      pch = strtok(NULL, ";");
      a++;
   }

   for (int i = 0; i < a; i++) {
      ////////////////PIPE & YONLENDIRME('>') ICEREN////////////////
      if (directionbool[i] == 2 && pipebool[i] == 1) {
         char * pchtemp;
         pch2 = strtok(programOutput[i], ">");
         int j = 0;
         while (pch2 != NULL) {
            pipeDirection[j] = pch2;

            pch2 = strtok(NULL, ">");
            j++;
         }
         int k = 0;
         pch2 = strtok(pipeDirection[0], "|");
         while (pch2 != NULL) {
            temp[k] = pch2;

            pch2 = strtok(NULL, "|");
            k++;
         }
         struct Commands commands[k];
         for (int l = 0; l < k; l++) {
            commands[l].background = 0;
            pch2 = strtok(temp[l], " ");
            int m = 0, counter = 0;
            while (pch2 != NULL) {
               if (m == 0) {
                  if ( * (pch2) == '.' && * (pch2 + 1) == '/') {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olmayıp & ile bitme durumu(arkaplan işlemi)
                        commands[l].background = 1;
                     }
                     commands[l].type = 0;
                     commands[l].process[0] = pch2 + 2;
                  } else {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olup  & ile bitme durumu(arkaplan işlemi)
                        commands[l].background = 1;
                     }
                     commands[l].type = 1;
                     commands[l].process[0] = pch2;
                  }
               } else if ( * pch2 == '&') {
                  // arkaplan olup olmaması =pch2;
                  pch2 = pchtemp;
                  commands[l].background = 1;
               } else if (m >= 1) {
                  if ( * (pch2 + strlen(pch2) - 1) == '&') {
                     *(pch2 + strlen(pch2) - 1) = '\0';
                     // Parametrenin & ile bitme durumu
                     commands[l].background = 1;
                  }
                  commands[l].process[1] = pch2;
                  commands[l].process[2] = NULL;
               }
               pchtemp = pch2;
               pch2 = strtok(NULL, " ");
               m++;
            }
            if (m == 1) {
               commands[l].process[1] = NULL;
            }
            counter++;
         }
         char file_name[20];
         int i1 = 1;
         int j2 = 0;
         if (pipeDirection[j - 1][0] == ' ') {
            while (pipeDirection[j - 1][i1] != '\0') {
               file_name[i1 - 1] = pipeDirection[j - 1][i1];
               i1++;
            }
         } else {
            i1--;
            while (pipeDirection[j - 1][i1] != '\0') {
               file_name[i1] = pipeDirection[j - 1][i1];
               i1++;
            }
         }
         file_name[i1 - 1] = '\0';
         if (Pipe_and_outputRd(commands, k, file_name) < 0) {
            return -1;
         }
      }
      ////////////////SADECE YONLENDIRME('<') ICEREN////////////////
      if (directionbool[i] == 1 && pipebool[i] == 0) {
         char * pchtemp;
         pch2 = strtok(programOutput[i], "<");
         int j = 0;
         while (pch2 != NULL) {
            pipeDirection[j] = pch2;

            pch2 = strtok(NULL, "<");
            j++;
         }
         struct Commands commands[j - 1];
         for (int l = 0; l < j - 1; l++) {
            commands[l].background = 0;
            pch2 = strtok(pipeDirection[l], " ");
            int m = 0, counter = 0;
            while (pch2 != NULL) {
               if (m == 0) {
                  if ( * (pch2) == '.' && * (pch2 + 1) == '/') {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olmayıp & ile bitme durumu(arkaplan işlemi)
                        commands[l].background = 1;
                     }
                     commands[l].type = 0;
                     commands[l].process[0] = pch2 + 2;
                  } else {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olup  & ile bitme durumu(arkaplan işlemi)
                        commands[l].background = 1;
                     }
                     commands[l].type = 1;
                     commands[l].process[0] = pch2;
                  }
               } else if ( * pch2 == '&') {
                  // arkaplan olup olmaması =pch2;
                  pch2 = pchtemp;
                  commands[l].background = 1;
               } else if (m >= 1) {
                  if ( * (pch2 + strlen(pch2) - 1) == '&') {
                     *(pch2 + strlen(pch2) - 1) = '\0';
                     // Parametrenin & ile bitme durumu
                     commands[l].background = 1;
                  }
                  commands[l].process[1] = pch2;
                  commands[l].process[2] = NULL;
               }
               pchtemp = pch2;
               pch2 = strtok(NULL, " ");
               m++;
            }
            if (m == 1) {
               commands[l].process[1] = NULL;
            }
            counter++;
         }
         char file_name[20];
         int i1 = 1;
         int j2 = 0;
         if (pipeDirection[j - 1][0] == ' ') {
            while (pipeDirection[j - 1][i1] != '\0') {
               file_name[i1 - 1] = pipeDirection[j - 1][i1];
               i1++;
            }
         } else {
            i1--;
            while (pipeDirection[j - 1][i1] != '\0') {
               file_name[i1] = pipeDirection[j - 1][i1];
               i1++;
            }
         }
         file_name[i1 - 1] = '\0';
         if (inputRD(commands, j - 1, file_name) < 0) {
            fprintf(stderr, "Dosyadan okunurken hata olustu\n");
            return -1;
         }
      }
      ////////////////SADECE YONLENDIRME('>')ICEREN////////////////
      if (directionbool[i] == 2 && pipebool[i] == 0) {
         char * pchtemp;
         pch2 = strtok(programOutput[i], ">");
         int j = 0;
         while (pch2 != NULL) {
            pipeDirection[j] = pch2;

            pch2 = strtok(NULL, ">");
            j++;
         }
         struct Commands command[j - 1];

         for (int l = 0; l < j - 1; l++) {
            command[l].background = 0;
            pch2 = strtok(pipeDirection[l], " ");
            int m = 0, counter = 0;
            while (pch2 != NULL) {
               if (m == 0) {
                  if ( * (pch2) == '.' && * (pch2 + 1) == '/') {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olmayıp & ile bitme durumu(arkaplan işlemi)
                        command[l].background = 1;
                     }
                     command[l].type = 0;
                     command[l].process[0] = pch2 + 2;
                  } else {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olup  & ile bitme durumu(arkaplan işlemi)
                        command[l].background = 1;
                     }
                     command[l].type = 1;
                     command[l].process[0] = pch2;
                  }
               } else if ( * pch2 == '&') {
                  // arkaplan olup olmaması =pch2;
                  command[l].background = 1;
                  pch2 = pchtemp;
               } else if (m >= 1) {
                  if ( * (pch2 + strlen(pch2) - 1) == '&') {
                     *(pch2 + strlen(pch2) - 1) = '\0';
                     // Parametrenin & ile bitme durumu
                     command[l].background = 1;
                  }
                  command[l].process[1] = pch2;
                  command[l].process[2] = NULL;
               }
               pchtemp = pch2;
               pch2 = strtok(NULL, " ");
               m++;
            }
            if (m == 1) {
               command[l].process[1] = NULL;
            }
            counter++;
         }
         char file_name[20];
         int i1 = 1;
         int j2 = 0;
         if (pipeDirection[j - 1][0] == ' ') {
            while (pipeDirection[j - 1][i1] != '\0') {
               file_name[i1 - 1] = pipeDirection[j - 1][i1];
               i1++;
            }
         } else {
            i1--;
            while (pipeDirection[j - 1][i1] != '\0') {
               file_name[i1] = pipeDirection[j - 1][i1];
               i1++;
            }
         }
         file_name[i1 - 1] = '\0';
         if (outputRD(command, j - 1, file_name) < 0) {
            fprintf(stderr, "Yonlednirmede hata olustu\n");
            return -1;
         }
      }
      ////////////////PIPE VE YONLENDIRME ICERMEYEN////////////////
      if (directionbool[i] == 0 && pipebool[i] == 0) {
         char * pchtemp;
         pch2 = strtok(programOutput[i], " ");
         int m = 0, counter = 0;
         struct Commands command;
         command.background = 0;
         char quit[5] = "quit";

         while (pch2 != NULL) {
            if (m == 0) //İlk girdiye  bakar ve her zaman bu işlem olmak zorundadır
            {
               if ( * (pch2) == '.' && * (pch2 + 1) == '/') {
                  if ( * (pch2 + strlen(pch2) - 1) == '&') {
                     *(pch2 + strlen(pch2) - 1) = '\0';
                     // sistem komutu olmayıp & ile bitme durumu(arkaplan işlemi)
                     command.background = 1;
                  }
                  command.type = 0;
                  command.process[0] = pch2 + 2;
               } else {
                  if ( * (pch2 + strlen(pch2) - 1) == '&') {
                     *(pch2 + strlen(pch2) - 1) = '\0';

                     // sistem komutu olup  & ile bitme durumu(arkaplan işlemi)
                     command.background = 1;
                  }
                  command.type = 1;
                  command.process[0] = pch2;
               }
            }
            if ( * pch2 == '&') {
               // arkaplan olup olmaması =pch2; (yukarıdakilerden farkı &'nin  işlemden boşluk
               // ile ayr0ılmış olması )
               pch2 = pchtemp;
               command.background = 1;
            }
            if (m >= 1) // Parametrelere bakar
            {
               if ( * (pch2 + strlen(pch2) - 1) == '&') {
                  *(pch2 + strlen(pch2) - 1) = '\0';
                  command.background = 1;

                  // Parametrenin & ile bitme durumunun
               }
               command.process[1] = pch2;
               command.process[2] = NULL;
            }
            pchtemp = pch2;
            pch2 = strtok(NULL, " ");
            m++;
         }
         if (m == 1) {
            command.process[1] = NULL;
            if (strstr(input, quit) != NULL) {
               while (1) {
                  if (NumberOfBackgroundProcess == 0)
                     exit(0);
               }
            }
         }
         counter++;
         if (Execute_Command(command) < 0) {
            fprintf(stderr, "Komutun yurutulmesinde hata olsutu\n");
            return -1;
         }
      }
      ////////////////SADECE PIPEICERENLER////////////////
      if (directionbool[i] == 0 && pipebool[i] == 1) {
         int k = 0;
         pch2 = strtok(programOutput[i], "|");
         while (pch2 != NULL) {
            temp[k] = pch2;
            pch2 = strtok(NULL, "|");
            k++;
         }

         struct Commands commands[k];
         for (int l = 0; l < k; l++) {
            pch2 = strtok(temp[l], " ");
            int m = 0, counter = 0, counter2 = 0;
            while (pch2 != NULL) {
               if (m == 0) {
                  if ( * (pch2) == '.' && * (pch2 + 1) == '/') {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                        // sistem komutu olmayıp & ile bitme durumu(arkaplan işlemi)
                     }
                     commands[l].type = 0;
                     commands[l].process[0] = pch2 + 2;
                  } else {
                     if ( * (pch2 + strlen(pch2) - 1) == '&') {
                        *(pch2 + strlen(pch2) - 1) = '\0';
                     }
                     commands[l].type = 1;
                     commands[l].process[0] = pch2;
                  }
               } else if (m >= 1) {
                  if ( * (pch2 + strlen(pch2) - 1) == '&') {
                     *(pch2 + strlen(pch2) - 1) = '\0';
                     // Parametrenin & ile bitme durumu
                  }
                  commands[l].process[1] = pch2;
                  commands[l].process[2] = NULL;
               } else if ( * pch2 == '&') {
                  // arkaplan olup olmaması =pch2;
               }
               pch2 = strtok(NULL, " ");
               m++;
            }
            if (m == 1) {
               commands[l].process[1] = NULL;
            }
            counter++;
         }
         if (Pipe(commands, k) < 0) {
            fprintf(stderr, "pipe olusurken hata!\n");
            return -1;
         }
      }
   }
}

void Prompt() {
   char hostname[HOST_NAME_MAX];
   char username[LOGIN_NAME_MAX];
   char cwd[1024];

   // getcwd su anki dosyanin konumunu getirir.x
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
      // hostname ve username
      getlogin_r(username, LOGIN_NAME_MAX);
      gethostname(hostname, HOST_NAME_MAX);

      fprintf(stderr, "\033[1;36m%s@%s:\033[1;35m~%s\033[0;33m>", username, hostname, cwd);
      // Bufferda tutulan output verileri temizlendi.
      fflush(stdout);
   }
}
int main() {
   char input[1024];
   NumberOfBackgroundProcess = 0;
   while (1) {
      Prompt();
      fgets(input, sizeof input, stdin);
      for (int i = 0; i < 1024; i++) {
         if (input[i] == '\n') {
            input[i] = '\0';
         }
         if (input[i] == '(' || input[i] == ')') {
            input[i] = ' ';
         }
      }
      Recognizer(input);
   }
   return 0;
}

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

/// Pipe icin coklu komutlarD1 kolay alabilmek icin struct yapisi

struct Commands
{
    char* prosses[3];
    int type;
};

/// Path degiskenini kullanan exec fonksiyonu

int Execvp(char* parameters[])
{
    if (execvp(parameters[0], parameters) < 0)
    {
        perror(parameters[0]);
        return -1;
    }
    return 0;
}

/// AynD1 dizindeki programlarD1 calistirmak icin exec fonksiyonu

int Execv(char* parameters[])
{
    if (execv(parameters[0], parameters) < 0)
    {
        perror(parameters[0]);
        return -1;
    }
    return 0;
}

int Pipe(struct Commands commands[], int number)
{
    int fd[2];
    pid_t pid;
    int fdd = READ_END;

    for (int i = 0; i < number; i++)
    {
        // fd C<zerinde pipe oluEturuldu
        pipe(fd);
        // Cocuk olusurken hata var mD1 diye kontrol edildi
        if ((pid = fork()) == -1)
        {
            perror("fork");
            return -1;
        }
        // Cocuk icinde yapD1lacak Eeyler buraya yazD1lD1r
        else if (pid == 0)
        {
            // Ebeveyn ile habarleEmek iC'in
            dup2(fdd, STDIN_FILENO);
            // Eger son komut degilse bilgisini bir sonraki komut iC'in yazma ucuna yazar
            if (i != (number - 1))
            {
                dup2(fd[WRITE_END], STDOUT_FILENO);
            }
            // KullanD1lmayan pipe lar kapatD1lD1r
            close(fd[READ_END]);
            // Eger girilmiE olan komut sistem komutu ise Execvp calistirilir
            if (commands[i].type == System_Call)
            {
                if (Execvp(commands[i].prosses) < 0)
                    return -1;
            }
            else
            {
                if (Execv(commands[i].prosses) < 0)
                {
                    return -1;
                }
            }
        }
        // Ebeveyn cocuklarD1 bekler
        else
        {
            wait(NULL);
            // KullanD1lmayan uc kapatD1lD1r
            close(fd[WRITE_END]);
            // Cocugun yazma ucu Ebeveynin okuma ucuna esitlenir
            fdd = fd[READ_END];
        }
    }
    return 0;
}

int Execute_Command(struct Commands command)
{
    pid_t pid;
    // Cocuk olusurken hata var mD1 diye kontrol edildi
    if ((pid = fork()) == -1)
    {
        perror("fork");
        return -1;
    }
    // Cocuk icinde yapD1lacak Eeyler buraya yazD1lD1r
    else if (pid == 0)
    {
        if (command.type == System_Call)
        {
            if (Execvp(command.prosses) < 0)
                return -1;
        }
        else
        {
            if (Execv(command.prosses) < 0)
            {
                return -1;
            }
        }
    }
    // Ebeveyn cocuklari bekler
    else
    {
        wait(NULL);
    }
    return 0;
}

//////KULLANICIDAN ALINAN GIRDININ AYIRILIP TANINMASI/////
int Recognizer(char* input)
{
    char *pch, *pch2, *pch3;
    char *programOutput[CHAR_MAX], *pipeDirection[CHAR_MAX], *temp[CHAR_MAX], *process[CHAR_MAX],
        *parameter[CHAR_MAX];
    ;
    int a = 0, pipebool[CHAR_MAX] = {}, directionbool[CHAR_MAX] = {};
    // KullanD1cD1dan alD1nan girdinin ';' iEaretine gC6re bloklara ayrD1lmasD1
    pch = strtok(input, ";");
    while (pch != NULL)
    {
        pch2 = pch;

        programOutput[a] = pch;
        // BloklarD1na ayrD1lan girdinin pipe ve yC6nlendirme iC'erme durumunun kontrolC<
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
        ////////////////////////////////////////////////PIPE & YONLENDIRME('<')
        ///ICEREN//////////////////////////////////////////
        if (directionbool[i] == 1 && pipebool[i] == 1)
        {
            // AlD1nan pointer dizinin < iEaretine gC6re bC6lC<nmesi ve her bC6lC<nen bloDun bir
            // pointer ile iEaretlenmesi
            pch2 = strtok(programOutput[i], "<");
            int j = 0;
            while (pch2 != NULL)
            {
                pipeDirection[j] = pch2;

                pch2 = strtok(NULL, "<");
                j++;
            }
            // pipeDirection[i-1]=FILE

            int k = 0;
            // AlD1nan pointer dizinin | iEaretine gC6re bC6lC<nmesive her bC6lC<nen bloDun bir
            // pointer ile iEaretlenmesi
            pch2 = strtok(pipeDirection[0], "|");
            while (pch2 != NULL)
            {
                temp[k] = pch2;

                pch2 = strtok(NULL, "|");
                k++;
            }
            struct Commands commands[k];
            for (int l = 0; l < k; l++)
            {
                // AlD1nan pointer dizinin boEluklarD1na  bC6lC<nmesi, her bC6lC<nen bloDun bir
                // pointer ile iEaretlenmesi ve sistem komutu olup olmadD1DD1nD1n
                // belirlenmesi.Sistem komutu deDil ise isminin dC<zenlenmesi
                pch2 = strtok(temp[l], " ");
                int m = 0, counter = 0;
                while (pch2 != NULL)
                {
                    if (m == 0)
                    { // "./" ile baElD1yor ise sistem komutu deDildir
                        if (*(pch2) == '.' && *(pch2 + 1) == '/')
                        {
                            commands[l].type = 0;
                            commands[l].prosses[0] = pch2 + 2;
                        }
                        else
                        {
                            commands[l].type = 1;
                            commands[l].prosses[0] = pch2;
                        }
                    }
                    else if (m == 1)
                    {
                        commands[l].prosses[1] = pch2;
                        commands[l].prosses[2] = NULL;
                    }
                    else if (m == 2 && *pch2 == '&')
                    {
                        // arkaplan olup olmaması =pch2;
                    }

                    pch2 = strtok(NULL, " ");
                    m++;
                }
                if (m == 1)
                {
                    commands[l].prosses[1] = NULL;
                }
                counter++;
            }
        }
        ////////////////////////////////////////////////PIPE & YONLENDIRME('>')
        ///ICEREN//////////////////////////////////////////
        if (directionbool[i] == 2 && pipebool[i] == 1)
        {
            pch2 = strtok(programOutput[i], ">");
            int j = 0;
            while (pch2 != NULL)
            {
                pipeDirection[j] = pch2;

                pch2 = strtok(NULL, ">");
                j++;
            }
            // pipeDirection[i-1]=FILE

            int k = 0;
            pch2 = strtok(pipeDirection[0], "|");
            while (pch2 != NULL)
            {
                temp[k] = pch2;

                pch2 = strtok(NULL, "|");
                k++;
            }
            struct Commands commands[k];
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
                            commands[l].type = 0;
                            commands[l].prosses[0] = pch2 + 2;
                        }
                        else
                        {
                            commands[l].type = 1;
                            commands[l].prosses[0] = pch2;
                        }
                    }
                    else if (m == 1)
                    {
                        commands[l].prosses[1] = pch2;
                        commands[l].prosses[2] = NULL;
                    }
                    else if (m == 2 && *pch2 == '&')
                    {
                        // arkaplan olup olmaması =pch2;
                    }

                    pch2 = strtok(NULL, " ");
                    m++;
                }
                if (m == 1)
                {
                    commands[l].prosses[1] = NULL;
                }
                counter++;
            }
        }
        ////////////////////////////////////////////////SADECE YONLENDIRME('<')
        ///ICEREN//////////////////////////////////////////
        if (directionbool[i] == 1 && pipebool[i] == 0)
        {
            pch2 = strtok(programOutput[i], "<");
            int j = 0;
            while (pch2 != NULL)
            {
                pipeDirection[j] = pch2;

                pch2 = strtok(NULL, "<");
                j++;
            }
            struct Commands commands[j];
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
                            commands[l].type = 0;
                            commands[l].prosses[0] = pch2 + 2;
                        }
                        else
                        {
                            commands[l].type = 1;
                            commands[l].prosses[0] = pch2;
                        }
                    }
                    else if (m == 1)
                    {
                        commands[l].prosses[1] = pch2;
                        commands[l].prosses[2] = NULL;
                    }
                    else if (m == 2 && *pch2 == '&')
                    {
                        // arkaplan olup olmaması =pch2;
                    }

                    pch2 = strtok(NULL, " ");
                    m++;
                }
                if (m == 1)
                {
                    commands[l].prosses[1] = NULL;
                }
                counter++;
            }
        }
        ////////////////////////////////////////////////SADECE YONLENDIRME('>')
        ///ICEREN//////////////////////////////////////////
        if (directionbool[i] == 2 && pipebool[i] == 0)
        {
            pch2 = strtok(programOutput[i], ">");
            int j = 0;
            while (pch2 != NULL)
            {
                pipeDirection[j] = pch2;

                pch2 = strtok(NULL, ">");
                j++;
            }
            struct Commands commands[j];
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
                            commands[l].type = 0;
                            commands[l].prosses[0] = pch2 + 2;
                        }
                        else
                        {
                            commands[l].type = 1;
                            commands[l].prosses[0] = pch2;
                        }
                    }
                    else if (m == 1)
                    {
                        commands[l].prosses[1] = pch2;
                        commands[l].prosses[2] = NULL;
                    }
                    else if (m == 2 && *pch2 == '&')
                    {
                        // arkaplan olup olmaması =pch2;
                    }

                    pch2 = strtok(NULL, " ");
                    m++;
                }
                if (m == 1)
                {
                    commands[l].prosses[1] = NULL;
                }
                counter++;
            }
        }
        ////////////////////////////////////////////////PIPE VE YONLENDIRME
        ///ICERMEYEN//////////////////////////////////////////
        if (directionbool[i] == 0 && pipebool[i] == 0)
        {
            pch2 = strtok(programOutput[i], " ");
            int m = 0, counter = 0;
            struct Commands command;
            while (pch2 != NULL)
            {
                if (m == 0)
                {
                    if (*(pch2) == '.' && *(pch2 + 1) == '/')
                    {
                        command.type = 0;
                        command.prosses[0] = pch2 + 2;
                    }
                    else
                    {
                        command.type = 1;
                        command.prosses[0] = pch2;
                    }
                }
                else if (m == 1)
                {
                    command.prosses[1] = pch2;
                    command.prosses[2] = NULL;
                }
                else if (m == 2 && *pch2 == '&')
                {
                    // arkaplan olup olmaması =pch2;
                }

                pch2 = strtok(NULL, " ");
                m++;
            }
            if (m == 1)
            {
                command.prosses[1] = NULL;
            }
            counter++;
            if (Execute_Command(command) < 0)
            {
                printf("Komutun yurutulmesinde hata olsutu\n");
                return -1;
            }
        }
        /////////////////////////////////////////////////SADECE PIPE
        ///ICERENLER/////////////////////////////////////////
        if (directionbool[i] == 0 && pipebool[i] == 1)
        {
            int k = 0;
            pch2 = strtok(programOutput[i], "|");
            while (pch2 != NULL)
            {
                temp[k] = pch2;
                pch2 = strtok(NULL, "|");
                k++;
            }

            struct Commands commands[k];
            for (int l = 0; l < k; l++)
            {
                pch2 = strtok(temp[l], " ");
                int m = 0, counter = 0, counter2 = 0;
                while (pch2 != NULL)
                {
                    if (m == 0)
                    {
                        if (*(pch2) == '.' && *(pch2 + 1) == '/')
                        {
                            commands[l].type = 0;
                            commands[l].prosses[0] = pch2 + 2;
                        }
                        else
                        {
                            commands[l].type = 1;
                            commands[l].prosses[0] = pch2;
                        }
                    }
                    else if (m == 1)
                    {
                        commands[l].prosses[1] = pch2;
                        commands[l].prosses[2] = NULL;
                    }
                    else if (m == 2 && *pch2 == '&')
                    {
                        // arkaplan olup olmaması =pch2;
                    }
                    pch2 = strtok(NULL, " ");
                    m++;
                }
                if (m == 1)
                {
                    commands[l].prosses[1] = NULL;
                }
                counter++;
            }
            if (Pipe(commands, k) < 0)
            {
                printf("pipe olusurken hata!\n");
                return -1;
            }
        }
    }
}

int Prompt()
{
    char hostname[HOST_NAME_MAX];
    char username[LOGIN_NAME_MAX];
    char cwd[1024];

    // getcwd Eu anki dosyanD1n konumunu getirir.x
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
    while (1)
    {
        Prompt();
        fgets(input, sizeof input, stdin);
        for (int i = 0; i < 1024; i++)
        {
            if (input[i] == '\n')
            {
                input[i] = '\0';
            }
        }
        // Sonsuz donguye girmesin diye hata olusur ise kontrol altina almak icin
        if (Recognizer(input) < 0)
        {
            break;
        }
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
 } */
    return 0;
}

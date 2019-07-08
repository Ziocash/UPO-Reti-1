#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h> 
#define MAX 512

const char *setComandi[] = {"START", "TEXT", "HIST", "QUIT", "EXIT"}; // set di comandi da eseguire

/* ELENCO FUNZIONI */
void select_menu();
void choose_menu(char buffer[MAX], int simpleSocket);
int string_count(char *str);
char split_text(char *str, size_t size);
void insert_text(char *text, int simpleSocket);
/*-------------------------------------------------*/


void select_menu() // opzioni del menu
{
    fprintf(stderr,"*************************************\n");
    fprintf(stderr,"[!] ESEGUIRE UNA DELLE SEGUENTI ISTRUZIONI\n");
    fprintf(stderr,"[1] INSERIMENTO TESTO\n");
    fprintf(stderr,"[2] ANALISI DEL TESTO\n");
    fprintf(stderr,"[3] USCITA DAL TESTO CON ANAISI DEL TESTO\n");
    fprintf(stderr,"[4] USCITA DAL PROGRAMMA SENSA ANALISI DEL TESTO\n");
    fprintf(stderr,"*************************************\n\n");
    fflush(stderr);
    
}

char split_text(char *str, size_t size) //mandare più stringhe se supera 512 byte
{
    char *p = NULL;
    char *temp;
    int i;
    if(str != NULL)
    {
        p = str;
    }

    if(p == NULL || *p == '\0')
    {
        return 0;
    }
    temp = (char*)malloc((size+1)*sizeof(char));
    for(i=0; *p && i<size; ++i)
    {
        temp[i]=*p++;
    }
    temp[i]='\0';
    return *temp;
}


int string_count(char *str)  //contatore stringa da inserire alla fine del messaggio da mandare
{
    int count = 0;
    int i = 0;
    for(i = 0; i < strlen(str); i++)
    {
        if(isdigit(str[i])||isalpha(str[i]))
        {
             count ++;
        }
    }

    return count;
}


void insert_text(char *text, int simpleSocket)
{
    char testo[MAX] = ""; //valore del testo
    text = testo; //appoggio puntatore
    char *temp[MAX]; //variabile d'appogio per split della string
    int i,j; //contatori cicli for split stringa
    char messaggio; //variabile messaggio da inviare al server per esempio: lato server: TEXT prova messaggio 14

    fprintf(stderr,"[!] Questa opzione permette di inserire un testo che succesivamente potrai analizzare.\n");
    fprintf(stderr,"[NB] La stringa sarà suddivisa in più sotto-stringhe in caso fosse più lunga di 512 .\n");
    fflush(stderr);

    fprintf(stderr,"[!] INSERISCI IL TESTO: ");
    //fflush(stdin);
    fgets(text, sizeof(testo), stdin);
    testo[strlen(testo) -1] = '\0';

    if(strlen(testo) > 512)
        {
            for (i = 0; *text=(split_text(text,512)) != '\0'; text= NULL)
            {
                temp[i++] = text;
            }
            for (j = 0; j < i;++j)
            {
                write(simpleSocket, temp, strlen(*temp));
            }
        }

    write(simpleSocket, text, strlen(testo));

    choose_menu(text,simpleSocket);

    
}

/*
void analyze_text()
{

}*/

void choose_menu(char buffer[MAX], int simpleSocket)
{
    int scelta = 0;
    select_menu();
    
    do
    {
        fprintf(stderr,"[!] SCEGLIERE UNA DELLE OPZIONI: ");
        fscanf(stdin," %d ", &scelta);
        fflush(stderr);

        switch (scelta)
        {
            case 1: //TEXT
            insert_text(buffer,simpleSocket);
            break;

            case 2: //HIST
            break;

            case 3: //EXIT
            break;

            case 4: //QUIT
            break;

            default: 
            fprintf(stderr,"[-] OPS! OPZIONE NON VALIDA!\n");
            fprintf(stderr,"[!] RIPROVA DI NUOVO\n\n");
            
        }
   } while(scelta > 0 || scelta < 5);

} 

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int ret = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[MAX] = "";
    struct sockaddr_in simpleServer;

     if (3 != argc) {

        fprintf(stderr,"[-] USARE IL FORMATO: %s <server> <port>\n", argv[0]);
        fflush(stderr);
        exit(1);

    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(simpleSocket < 0){
        fprintf(stderr,"[+] ERRORE DI CONNESSIONE\n\n");
        fflush(stderr);
        exit(1);
    }
    fprintf(stderr,"[+] CLIENT SOCKET CREATO CORRETTAMENTE\n\n");
    fflush(stderr);

    simplePort = atoi(argv[2]);

    memset(&simpleServer, 0 , sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    ret = connect(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));
    
    if(ret < 0)
    {
        fprintf(stderr,"[-] ERRORE DI CONNESSIONE\n\n");
        fflush(stderr);
        exit(1);
    }
    fprintf(stderr,"[+] CONNESSIONE AL SERVER CREATA\n\n");
    fflush(stderr);
    
    choose_menu(buffer, simpleSocket);

    

    

    if(read(simpleSocket, buffer, 512) < 0){
        fprintf(stderr,"[-] ERRORE NELLA RECEZIONE DEI DATI\n");
        fflush(stderr);
        close(simpleSocket);
        
    } 

    return 0;
}
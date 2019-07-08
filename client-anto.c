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
void choose_menu(char buffer[512], int simpleSocket);
int string_count(char *str);
//char split_text(char *str, size_t size);
void insert_text(char *text, int simpleSocket);
/*-------------------------------------------------*/


void select_menu() // opzioni del menu
{
    fprintf(stderr,"*************************************\n");
    fprintf(stderr,"[!]ESEGUIRE UNA DELLE SEGUENTI ISTRUZIONI\n");
    fprintf(stderr,"[1]INSERIMENTO TESTO\n");
    fprintf(stderr,"[2]ANALISI DEL TESTO\n");
    fprintf(stderr,"[3]USCITA DAL TESTO CON ANAISI DEL TESTO\n");
    fprintf(stderr,"[4]USCITA DAL PROGRAMMA SENSA ANALISI DEL TESTO\n");
    fprintf(stderr,"*************************************\n\n");
    fflush(stderr);
    
}

/*void send_split_string(char text, int simpleSocket)
{

}*/


int string_count(char *str)  //contatore stringa da inserire alla fine del messaggio da mandare
{
    int count = 0;
    int i = 0;
    for(i = 0; i < strlen(str); i++)
    {
        if(isdigit(str[i])||isalpha(str[i]))
        {
             count++;
        }
    }

    return count;
}


void insert_text(char *text, int simpleSocket)
{
    char testo[500] = ""; //valore del testo
    text = testo; //appoggio puntatore
    char messaggio[MAX];
    bzero(messaggio, sizeof(messaggio));    

    fprintf(stderr,"[!]Questa opzione permette di inserire un testo che succesivamente potrai analizzare.\n");
    fprintf(stderr,"[NB]La stringa sarà suddivisa in più sotto-stringhe in caso fosse più lunga di 512 .\n");
    fprintf(stderr,"[!]INSERISCI IL TESTO: ");
    fscanf(stdin, " %[^\n]s\n", text);
    int lengthStr = string_count(testo);
    if (strlen(testo) > 512)
    {

    } else
    {
        char dim[20];
        bzero(dim, sizeof(dim));
        strcat(testo, " ");
        sprintf(dim, "%d", lengthStr); //da int a stringhissimo!
        strcat(text, dim);
        strcat(messaggio, setComandi[1]);
        strcat(messaggio, " ");
        strcat(messaggio, text);    
        write(simpleSocket, messaggio, strlen(messaggio));
    }
    
    choose_menu(testo,simpleSocket);

    
}

/*
void analyze_text()
{

}*/

void choose_menu(char messaggio[512], int simpleSocket)
{
    int scelta = 0;

    select_menu();
    
    do
    {
        fprintf(stderr,"[!]SCEGLIERE UNA DELLE OPZIONI: ");
        fscanf(stdin,"%d", &scelta);
        fflush(stderr);

        switch (scelta)
        {
            case 1: //TEXT
            insert_text(messaggio,simpleSocket);

            break;

            case 2: //HIST
            break;

            case 3: //EXIT
            break;

            case 4: //QUIT
            break;

            default: 
            fprintf(stderr,"[-]OPS! OPZIONE NON VALIDA!\n");
            fprintf(stderr,"[!]RIPROVA DI NUOVO\n\n");
            
        }
   } while(scelta > 0 || scelta < 5);

} 

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int ret = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[512] = "";
    struct sockaddr_in simpleServer;

     if (3 != argc) {

        fprintf(stderr,"[-]USARE IL FORMATO: %s <server> <port>\n", argv[0]);
        fflush(stderr);
        exit(1);

    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(simpleSocket < 0){
        fprintf(stderr,"[+]ERRORE DI CONNESSIONE\n\n");
        fflush(stderr);
        exit(1);
    }
    fprintf(stderr,"[+]CLIENT SOCKET CREATO CORRETTAMENTE\n\n");
    fflush(stderr);

    simplePort = atoi(argv[2]);

    memset(&simpleServer, 0 , sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    ret = connect(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));
    
    if(ret < 0)
    {
        fprintf(stderr,"[-]ERRORE DI CONNESSIONE\n\n");
        fflush(stderr);
        exit(1);
    }
    fprintf(stderr,"[+]CONNESSIONE AL SERVER CREATA\n\n");
    fflush(stderr);
    
    choose_menu(buffer, simpleSocket);


    if(read(simpleSocket, buffer, 512) < 0){
        fprintf(stderr,"[-]ERRORE NELLA RECEZIONE DEI DATI\n");
        fflush(stderr);
        close(simpleSocket);
        
    } 

    return 0;
}
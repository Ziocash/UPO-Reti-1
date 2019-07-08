/* Applicazione di rete, costituita da un programma server ed un programma client, in cui il programma server calcola 
la distribuzione dei caratteri (diversi da spazi) che compongono un testo fornito dal client, secondo le specifiche 
indicate.                                                        */


#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <ctype.h>

const char MESSAGE[] = "OK START Benvenuto sul server!\n";

void append(char* s, char c);
void getLenght(char len[4], char *buff);
int strIsValid(char str[512]);
int count_characters(char str[]);
void writeHistBuffer(char str1[4096], char str2[512], size_t l);
void histRisposta(const char str1[4096], char str2[]);

//Funzione append
void append(char *s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
}

//Prende il contatore dalla fine del TEXT
void getLenght(char len[4], char *buff) {
    size_t i = 0;
    if(strlen(buff) <= 100)
        i = strlen(buff) - 4;
    else
        i = strlen(buff) - 5;
    
    int j = 0;

    while(*buff++ != '\n') {
        if(isdigit(buff[i])) {
            len[j] = buff[i];
            j++; 
        }       
    }
}

//Controlla se il testo è valido
int strIsValid(char str[512]) {
    int i;
    if(str[strlen(str) - 1] != '\n' || (strncmp(str, "TEXT", 4) != 0 && strncmp(str, "HIST", 4) != 0 && strncmp(str, "EXIT", 4) != 0 && strncmp(str, "QUIT", 4) != 0))
        return 0;

    for(i = 0;str[i] != '\0';i++)
    { 
        if(isalnum(str[i]) == 0 && (str[i] != ' ') && str[i] != '\n') {
            return 0;
        }        
    }
    return 1;
}

//Conta i caratteri del testo senza contare gli spazzi
int count_characters(char str[]) {
    int count = 0;
    for(size_t i = 0;i < strlen(str);i++)
        if(str[i] != ' ')
            count++; 

    return count;
}

//Scrive sul buffer utilizzato dal comando HIST
void writeHistBuffer(char str1[4096], char str2[512], size_t l) {
    size_t j = strlen(str1);
    if(strlen(str1) > 4000)
        return;
    for(size_t i = 5;i < strlen(str2) - l;i++) {
        if(str2[i] != ' ' && isalnum(str2[i])) {
            str1[j++] = str2[i];
        }
    }
    str1[strlen(str1)] = '\0';
    //printf("%s\n", str1);
}

void histRisposta(const char str1[4096], char str2[]) {
    char container[50] = "";     //Vengono inserite le lettere, in ordine che sono state trovate
    int countChar[50];    //Vengono inseriti i contatori, di ogni lettera, contenuti nello stesso indice di container 
    char len[5];
    int j = 0, x = 0;

    memset(countChar, 0, sizeof(countChar));
    memset(container, 0, sizeof(container));
    memset(len, 0, sizeof(len));

    for(size_t i = 0; str1[i] != '\0';i++) { 
        if(strchr(container, str1[i]) == NULL) {    
            append(container, str1[i]);
            countChar[x++] = 1;
        } else {
            while(container[j++] != str1[i]);
            countChar[j-1] += 1;
            j = 0;
        }
    }
    container[strlen(container)] = '\0'; 
    j = 0;      
    while(container[j] != '\0') {
        sprintf(len, "%d", countChar[j]);
        append(str2, container[j]);
        strcat(str2, ":");
        strcat(str2, len);
        strcat(str2, " ");
        memset(len, 0, sizeof(len));
        j++;
    }
    str2[strlen(str2) - 1] = '\0';
}

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    if (2 != argc) {

        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);

    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {

        fprintf(stderr, "Could not create a socket!\n");
        exit(1);

    }
    else {
	    fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer)); 
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    if (returnStatus == 0) {
	    fprintf(stderr, "Bind completed!\n");
    }
    else {
        fprintf(stderr, "Could not bind to address!\n");
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Cannot listen on socket!\n");
	    close(simpleSocket);
        exit(1);
    }

    struct sockaddr_in clientName = {
        0
    };
    int simpleChildSocket = 0;
    unsigned clientNameLength = sizeof(clientName);


    /*Inizio ascolto server*/
    char recivedBuffer[512] = "";
    char sendBuffer[512] = "";
    char histBuffer[4096] = "";
    char risposta[512] = "";    //Per il comando HIST e EXIT

    while (1) {
        memset(&histBuffer, 0, sizeof(histBuffer));
        memset(&recivedBuffer, 0, sizeof(recivedBuffer)); 
        memset(&sendBuffer, 0, sizeof(sendBuffer)); 

        int flag = 1;
        
        printf("\nIn ascolto di una richiesta...\n\n");

        simpleChildSocket = accept(simpleSocket, (struct sockaddr * ) & clientName, & clientNameLength);
        if (simpleChildSocket == -1) {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);            
        }
        write(simpleChildSocket, MESSAGE, strlen(MESSAGE));


        while (flag) {

            memset(&recivedBuffer, 0, sizeof(recivedBuffer)); 
            memset(&sendBuffer, 0, sizeof(sendBuffer));
            memset(risposta, 0, sizeof(risposta)); 

            returnStatus = read(simpleChildSocket, recivedBuffer, sizeof(recivedBuffer));
            if(returnStatus == 0) {
                flag = 0;                
            } else {
                char textLength[4] = "";
                getLenght(textLength, recivedBuffer);
                
                /******************** Scelta del comando ********************/
                //Prima di tutto controllo che la stringa inserita' sia corretta
                if(!strIsValid(recivedBuffer)) {
                    strcat(sendBuffer, "ERR SYNTAX - Il messaggio non e' sintatticamente corretto!\n");
                    flag = 0;
                }
                //*******       TEXT        *******//
                else if(strncmp(recivedBuffer, "TEXT", 4 ) == 0) {                    
                    writeHistBuffer(histBuffer, recivedBuffer, strlen(textLength) + 1);

                    if(count_characters(recivedBuffer) - 5 - (int)strlen(textLength) == atoi(textLength)) {
                        strcat(sendBuffer, "OK TEXT ");
                        strcat(sendBuffer, textLength);
                        strcat(sendBuffer, "\n");
                    } else {
                        strcat(sendBuffer, "ERR TEXT - Il messaggio non e' semanticamente corretto!\n");
                    }
                } 
                //*******       HIST        *******//
                else if(strncmp(recivedBuffer, "HIST", 4) == 0) {
                    histRisposta(histBuffer, risposta);
                    strcat(sendBuffer, "OK HIST ");
                    strcat(sendBuffer, risposta);
                    strcat(sendBuffer, "\n");
                    write(simpleChildSocket, sendBuffer, strlen(sendBuffer));
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strcat(sendBuffer, "OK HIST END\n");
                }
                //*******       EXIT        *******//
                else if(strncmp(recivedBuffer, "EXIT", 4) == 0) {                    
                    histRisposta(histBuffer, risposta);
                    strcat(sendBuffer, "OK HIST ");
                    strcat(sendBuffer, risposta);
                    strcat(sendBuffer, "\n");
                    write(simpleChildSocket, sendBuffer, strlen(sendBuffer));
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strcat(sendBuffer, "OK HIST END\n");
                    strcat(sendBuffer, "OK EXIT Connessione chiusa\n");  
                    flag = 0;
                }
                //*******       QUIT        *******//
                else if(strncmp(recivedBuffer, "QUIT", 4) == 0) {
                    strcat(sendBuffer, "OK QUIT Connessione chiusa!\n");
                    flag = 0;

                }
                write(simpleChildSocket, sendBuffer, strlen(sendBuffer));
            }  
        }
        close(simpleChildSocket);
    }   
    close(simpleSocket);
    return 0;

}


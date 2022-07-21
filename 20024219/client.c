#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define ATTEMPT 1
#define INVALID -1
#define PERFECT 0

char clt_msg[256] = "";
char buffer[2000] = " ";
int total_attempts = 0;
int actual_attempts = 0;
// int flag = 0;

/*
Funzione per il riconoscimento del comando OK

Ritorna la validità parziale del messaggio
*/
int ok_processing(const char *message, int *attempts, int *out_index)
{
    char string_attempts[3] = ""; // stringa per salvare i tentativi in arrivo dal server
    char cmd[3] = "";             // comando ricevuto (solo OK)
    int index = 0;                // indice di appoggio per cmd
    int spaces = 0;               // numero di spazi (ammesso solo 2 per correttezza messaggio) "OK 6 " oppure "OK 10 "
    for (int i = 0; i < 4; i++)
    {
        if (i < 2 && isalpha(message[i])) //"OK"
            cmd[index++] = message[i];
        else if (i == 2 && isspace(message[i])) // controllo il primo spazio " "
            spaces++;
        else if (i == 3 && isdigit(message[i])) // controlla numero
        {
            string_attempts[0] = message[i]; // salva prima cifra
            if (isdigit(message[i + 1]))     // controlla se successiva numero
            {
                string_attempts[1] = message[i + 1];
                if (isspace(message[i + 2]))
                {
                    *attempts = atoi(string_attempts);
                    *out_index = i + 3;
                    spaces++;
                    if (spaces == 2 && strcmp(cmd, "OK") == 0)
                        return ATTEMPT;
                }
            }
            else if (isspace(message[i + 1])) // controlla se sucessiva spazio
            {
                spaces++;
                *attempts = atoi(string_attempts);
                *out_index = i + 2;
                if (spaces == 2 && strcmp(cmd, "OK") == 0)
                    return ATTEMPT;
            }
        }
        else if (i == 3 && message[i] == 'P')
        {
            if (strcmp(message, "OK PERFECT\n") == 0)
                return PERFECT;
        }
        else
            return INVALID;
    }
    return INVALID;
}

/*
Funzione per il riconoscimento del comando QUIT

Ritornat la validità del messaggio
*/
int quit_processing(const char *message, int *out_index)
{
    char cmd[5] = "";
    for (int i = 0; i < 5; i++)
    {
        if (isalpha(message[i]))
        {
            cmd[i] = message[i];
            *out_index = i;
        }
        else if (isspace(message[i]))
            *out_index = i + 1;
        else
            return 0;
    }
    if (strcmp(cmd, "QUIT") == 0)
        return 1;
    return 0;
}

/*
Funzione per il riconoscimento del comando ERR

Ritornat la validità del messaggio
*/
int err_processing(const char *message, int *out_index)
{
    char cmd[4] = "";
    for (int i = 0; i < 4; i++)
    {
        if (isalpha(message[i]))
        {
            cmd[i] = message[i];
            *out_index = i;
        }
        else if (isspace(message[i]))
            *out_index = i + 1;
        else
            return 0;
    }
    if (strcmp(cmd, "ERR") == 0)
        return 1;
    return 0;
}

/*
Funzione per il riconoscimento del comando END

Ritornat la validità del messaggio
*/
int end_processing(const char *message, int *out_index)
{
    char cmd[4] = "";             // comando ricevuto (solo END)
    int index = 0;                // indice di appoggio per cmd
    int spaces = 0;               // numero di spazi (ammesso solo 2 per correttezza messaggio) "END 6 " oppure "END 10 "
    for (int i = 0; i < 5; i++)
    {
        if (i < 3 && isalpha(message[i])) //"END"
            cmd[index++] = message[i];
        else if (i == 3 && isspace(message[i])) // controllo il primo spazio " "
            spaces++;
        else if(message[strlen(message) - 1] == '\n' && strcmp(cmd, "END") == 0) 
            return 1;
        else
            return 0;
    }
    return 0;
}

/*
Funzione per stampare una stringa partendo dall'indice dato
*/
void print_message_from_index(const char *message, int start_index)
{
    for (int i = start_index; i < strlen(message); i++)
        printf("%c", message[i]);
}

/*
Invia comando WORD con la parola inserita nel messaggio
*/
void send_word_to_socket(int socket, const char *word)
{
    memset(buffer, '\0', 512 * sizeof(char));
    sprintf(clt_msg, "WORD %s\n", word);
    send(socket, clt_msg, strlen(clt_msg), 0);
}

int main(int argc, char *argv[])
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;

    struct sockaddr_in simpleServer;

    if (3 != argc)
    {

        fprintf(stderr, "Utilizzare: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1)
    {

        fprintf(stderr, "Impossibile creare una Socket!\n");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Socket creata!\n");
    }

    /* retrieve the port number for connecting */
    simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */

    memset(buffer, 0, 512 * (sizeof buffer[0]));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        fprintf(stderr, "Connessione riuscita!\n");
    }
    else
    {
        fprintf(stderr, "Impossibile connettersi all'indirizzo!\n");
        close(simpleSocket);
        exit(1);
    }

    /* get the message from the server   */
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));

    if (returnStatus > 0)
    {
        int index = 0;
        if (ok_processing(buffer, &total_attempts, &index))
            print_message_from_index(buffer, index);
    }
    else
    {
        fprintf(stderr, "Return Status = %d \n", returnStatus);
    }

    /* Parola da inviare al Server, il quale si trova in attesa */

    int close_flag = 0;

    while (close_flag == 0)
    {
        char parola[10] = "";
        int valid = 0;
        do
        {
            printf("Per uscire digitare <exit>.\n");
            printf("Inserisci la parola da inviare: ");
            scanf("%s", parola);
            if (strlen(parola) != 5)
            {
                if (strcmp(parola, "exit") == 0)
                    valid = 1;
                else
                    printf("Comando \"%s\" non riconosciuto. Ammesso solo il comando \'exit\'.\n", parola);
            }
            else
                valid = 1;
        } while (!valid);

        if (strlen(parola) == 5 && strcmp(parola, "exit") != 0)
            send_word_to_socket(simpleSocket, parola);
        else if (strcmp(parola, "exit") == 0)
            send(simpleSocket, "QUIT\n", strlen("QUIT\n"), 0);
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));

        /*stati di chiusura*/
        if (returnStatus == 0)
        {
            printf("Disconnesso.\n"); // in caso in cui il Server crashasse
            printf("%s\n", buffer);
            close(simpleSocket);
            exit(0);
        }
        else if (returnStatus == -1)
        {
            printf("Il Server ha chiuso la connessione.\n"); // in caso in cui il Server crashasse
            printf("%s\n", buffer);
            close(simpleSocket);
            exit(-1);
        }

        int result = 0;
        int index = 0;
        if ((result = ok_processing(buffer, &actual_attempts, &index)) == ATTEMPT)
        {
            printf("Non hai indovinato, ecco un indizio: ");
            print_message_from_index(buffer, index);
            printf("Tentativo numero: %d\n", actual_attempts);
        }
        else if (result == PERFECT)
        {
            printf("Hai indovinato la parola.\n");
            close(simpleSocket);
            exit(0);
        }
        else if (quit_processing(buffer, &index))
        {
            print_message_from_index(buffer, index);
            exit(0);
        }
        else if (err_processing(buffer, &index))
        {
            print_message_from_index(buffer, index);
            exit(-1);
        }
        else if(end_processing(buffer, &index))
        {
            printf("Hai finito i tentativi (%d). ", total_attempts);
            print_message_from_index(buffer, index);          
            exit(0);
        }
        else
        {
            printf("Messaggio malformato in arrivo dal server.\n");
            exit(-1);
        }

        memset(buffer, '\0', sizeof(buffer));
        memset(clt_msg, '\0', sizeof(clt_msg));
    }

    close(simpleSocket);
    return 0;
}

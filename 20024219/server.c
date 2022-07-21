#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h> //per la funzione rand

char srv_msg[256] = "";
char clt_msg[2000];
int limit;			 // massimo di tentativi possibili
char limite_def[10]; // dimensione possibilita
char limite_def2[10];
char delim[] = " ";
int simpleChildSocket = 0;
int simpleSocket = 0;
int simplePort = 0;
int returnStatus = 0;
struct sockaddr_in simpleServer;
char *lista_parole[] = {"conci", "fessa", "parla", "testo", "porto"}; // potrei metterne quante ne voglio, devo solo ricordare di modificare il num rand() e le parole devono essere di 5 lettere, ne di meno, ne di piu
int turno = 0;														  // 6 possibilita per indovinare
int close_socket = 0;

/*
Funzione per accogliere il client appena connesso
*/
void welcome_client(struct sockaddr_in clientName, int socket)
{
	char messaggio[60] = "";
	strcat(srv_msg, "OK");
	strcat(srv_msg, delim);
	sprintf(limite_def, "%d", limit);
	strcat(srv_msg, limite_def);
	strcat(srv_msg, delim);
	char address[16] = "";
	inet_ntop(AF_INET, &(clientName.sin_addr), address, INET_ADDRSTRLEN);
	printf("%s\n", address);
	sprintf(messaggio, "Benvenuto %s. Indovina la parola\n", address);
	strcat(srv_msg, messaggio);
	send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
	memset(srv_msg, 0, sizeof(srv_msg));
}

/*
 Funzione per processare i comandi in arrivo dal client

 Ritorna se il comando ricevuto è composto da soli caratteri oppure no
*/
int command_processing(char *message, char *cmd)
{
	int index = 0;
	for (int i = 0; i < 4; i++)
	{
		if (isalpha(message[i]))
			cmd[index++] = message[i];
		else
			return 0;
	}
	return 1;
}

/*
 Funzione per processare la parola dopo il comando WORD

 Ritorna se il messaggio ricevuto è corretto oppure no
 */
int process_word_command(char *message, char *word)
{
	int spaces = 0;
	char string[20] = "";
	int index = 0;
	for (int i = 4; i < strlen(message); i++)
		string[index++] = message[i];

	if (strlen(string) == 7)
		if (isspace(string[0]))
		{
			spaces++;
			for (int i = 1; i < strlen(string) - 1; i++)
			{
				if (isspace(string[i]) || !isalpha(string[i]))
					return 0;
				word[i - 1] = string[i];
			}
			if (string[strlen(string) - 1] == '\n')
				return 1;
		}
	return 0;
}

/*
Funzione per la comparazione delle parole

Ritorna il risultato in stringa, NULL se ci sono errori durante l'allocazione
*/
char *compare_word(const char *actual, const char *expected)
{
	char *result = (char *)calloc(6, sizeof(char));

	if (result == NULL)
		return NULL;

	sprintf(result, "-----");

	for (int i = 0; i < strlen(expected); i++)
	{
		if (expected[i] == actual[i])
			result[i] = '*';
		else
			for (int j = 0; j < strlen(actual); j++)
				if (expected[i] == actual[j])
					result[i] = '+';
	}
	return result;
}

void quit_response(const char *word, int socket)
{
	char message[256] = "";
	sprintf(message, "QUIT Vai via cosi' presto? La parola era %s\n", word);
	send(socket, message, strlen(message), 0);
}

void err_response(char *string, const char *keyword)
{
	strcat(string, "ERR ");
	strcat(string, "Il comando inviato risulta");
	strcat(string, delim);
	strcat(string, keyword);
	strcat(string, "\n");
}

void ok_response(char *string, const char *value)
{
	char tentativi_totali[3] = "";
	strcat(string, "OK ");
	sprintf(tentativi_totali, "%d", turno);
	strcat(string, tentativi_totali);
	strcat(string, delim);
	strcat(string, value);
	strcat(string, "\n");
}

void end_response(char *string, const char *word)
{
	strcat(string, "END");
	strcat(string, delim);
	strcat(string, "La parola era");
	strcat(string, delim);
	strcat(string, word);
	strcat(string, "\n");
}

int main(int argc, char *argv[])
{

	if (3 != argc && 2 != argc)
	{
		fprintf(stderr, "Utilizzare: %s <porta> [<tentativi>]\n", argv[0]);
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (simpleSocket == -1)
	{

		fprintf(stderr, "Impossibile creare una Socket!\n");
		exit(1);
	}
	else
	{
		fprintf(stderr, "Socket creata!\n");

		/* codice prelevato da un mio esercizio precedente, per bellezza estetica */
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		printf("Avvio Server: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	/* retrieve the port number for listening */
	simplePort = atoi(argv[1]);
	if (argc == 3)
		limit = atoi(argv[2]);
	else
		limit = 6;

	/* setup the address structure */
	/* use INADDR_ANY to bind to all local addresses  */
	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);

	/*  bind to the address and port with our socket  */
	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if (returnStatus == 0)
	{
		fprintf(stderr, "Bind completato!\n");
		fprintf(stderr, "Tentativi massimi: %d!\n", limit);
	}
	else
	{
		fprintf(stderr, "Impossibile associare all'indirizzo!\n");
		close(simpleSocket);
		exit(1);
	}

	/* lets listen on the socket for connections */
	returnStatus = listen(simpleSocket, 5);

	if (returnStatus == -1)
	{
		fprintf(stderr, "Impossibile ascoltare la Socket!\n");
		close(simpleSocket);
		exit(1);
	}

	while (1)
	{
		struct sockaddr_in clientName = {0};
		int clientNameLength = sizeof(clientName);

		/* wait here */
		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

		// printf("Il client si è collegato!\n");
		// codice prelevato da un mio vecchio compito, per estetica/log
		time_t now = time(NULL);
		printf("Un nuovo client si è collegato: %s", asctime(gmtime(&now)));
		welcome_client(clientName, simpleChildSocket);

		if (simpleChildSocket == 0)
		{

			fprintf(stderr, "Impossibile accettare connessioni!\n");
			close(simpleSocket);
			exit(1);
		}

		/* Server deve prelevare una parola random tra quelle presenti nell'array */
		srand(time(NULL));						 // lo utilizzo per la generazione dei numeri casuali
		char *parola = lista_parole[rand() % 5]; // esempio 1 su 5 possibilita definite, posso incrementare in base a quante parole io inserisco nell'array

		close_socket = 0;

		while (close_socket == 0)
		{
			memset(srv_msg, 0, sizeof(srv_msg));
			memset(clt_msg, 0, sizeof(clt_msg));
			recv(simpleChildSocket, clt_msg, sizeof(clt_msg), 0);

			char cmd[5] = "";
			char word[10] = "";

			if (command_processing(clt_msg, cmd))
			{
				fprintf(stdout, "Comando ricevuto: |%s|\n", cmd);
				if (strcmp(cmd, "WORD") == 0) // WORD ricevuta
				{
					if (process_word_command(clt_msg, word))
					{
						char *result = compare_word(word, parola);
						if (strcmp(result, "*****") == 0 && turno < limit) // indovinata
						{
							send(simpleChildSocket, "OK PERFECT\n", strlen("OK PERFECT\n"), 0);
							close_socket = 1;
						}
						else // non indovinata
						{
							++turno;
							if (turno < limit)
							{								
								ok_response(srv_msg, result);
								send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
							}
							else
							{
								end_response(srv_msg, parola);
								send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
								close_socket = 1;
							}
						}
						free(result);
					}
					else
					{
						err_response(srv_msg, "errato");
						send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
						close_socket = 1;
					}
				}
				else if (strcmp(cmd, "QUIT") == 0) // QUIT ricevuta
				{
					if (clt_msg[strlen(cmd)] == '\n')
					{
						quit_response(parola, simpleChildSocket);
					}
					else // Errore, comando non corretto
					{
						err_response(srv_msg, "non corretto");
						send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
					}
					close_socket = 1;
				}
				else // Errore, comando non consentito
				{
					err_response(srv_msg, "non consentito");
					send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
					close_socket = 1;
				}
			}
			else // Errore, comando non corretto
			{
				err_response(srv_msg, "non corretto");
				send(simpleChildSocket, srv_msg, strlen(srv_msg), 0);
				close_socket = 1;
			}
		}

		turno = 0;
		close(simpleChildSocket);
	}
}
//////////////////////////////////////////////////////////////////
// File name : cli.c						//
// Date : 2020.06.05						//
// OS : Ubuntu 16.04 64bits					//
// Author : Jeon Seung Hyeon					//
// Student ID : 2018202038					//
// ------------------------------------------------------------	//
// Title : System Programming Assignment #3			//
// Description : client						//
//////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define BUF_SIZE 1024

void sig_handler(int signo);	// signal handler

char buff2[BUF_SIZE];	// buff2 is a converted command and argument.

void sigalrm_handler(int signo);

//////////////////////////////////////////////////////////////////
// main								//
// ============================================================ //
// Input : int -> argc						//
//	   char** -> argv					//
// Output : Nothing						//
//								//
// Purpose : client						//
//////////////////////////////////////////////////////////////////
void main(int argc, char** argv) {

	char buff1[BUF_SIZE];			// buff1 is input written by the user.
	char rcv_buff[BUF_SIZE];		// server gives result to the client by writting it on rcv_buff
	int n;
	int sockfd;
	char *ch;
	struct sockaddr_in serv_addr;

	signal(SIGINT, (void *)sig_handler);

	signal(SIGALRM, sigalrm_handler);

	// open socket and connect
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	alarm(10);

	while (1) {
		memset(buff1, 0, BUF_SIZE);
		memset(buff2, 0, BUF_SIZE);
		memset(rcv_buff, 0, BUF_SIZE);
		write(STDOUT_FILENO, "> ", sizeof("> "));

		// input to be handed in to the server
		for (int i = 0; i < BUF_SIZE; i++) {
			n = getchar();
			if (n == '\n')
				break;
			else
				buff1[i] = n;
		}

		// SIGINT is implemented before?
		if (strcmp(buff2, "QUIT")) {

			ch = strtok(buff1, " \0");		
		
			// User command -> FTP command
			// pwd : Print Current Working Directory. pwd -> PWD
			if (!strcmp(ch, "pwd")) {
				strcpy(buff2, "PWD");
	
				// more argument exists : wrong input
				if ((ch = strtok(NULL, " \0")) != NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
			}

			// ls : Name List. ls -> NLST
			else if (!strcmp(ch, "ls")) {
				strcpy(buff2, "NLST");

				// more argument exists : wrong input
				if ((ch = strtok(NULL, " \0")) != NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
			}

			// dir : Directory Entry List. dir -> LIST
			else if (!strcmp(ch, "dir")) {
				strcpy(buff2, "LIST");

				// more argument exists : wrong input
				if ((ch = strtok(NULL, " \0")) != NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
			}

			// quit : terminate the command connection. quit -> QUIT
			else if (!strcmp(ch, "quit")) {
				strcpy(buff2, "QUIT");

				// more argument exists : wrong input
				if ((ch = strtok(NULL, " \0")) != NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
				// proper input
				else
					write(sockfd, buff2, BUF_SIZE);
			}	

			// cd : change working directory. cd -> CWD
			else if (!strcmp(ch, "cd")) {
				strcpy(buff2, "CWD ");

				// no argument
				if ((ch = strtok(NULL, " \0")) == NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
				// argument exists.
				else {
					strcat(buff2, ch);

					// more argument exists : wrong input
					if ((ch = strtok(NULL, " \0")) != NULL) {
						memset(buff2, 0, BUF_SIZE);
						strcpy(buff2, "ERROR");
					}
				}
			}

			// delete : remove file. delete -> DELE
			else if (!strcmp(ch, "delete")) {
				strcpy(buff2, "DELE ");
			
				// no argument
				if ((ch = strtok(NULL, " \0")) == NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
				// argument exists.
				else {
					strcat(buff2, ch);

					// more argument exists : wrong input
					if ((ch = strtok(NULL, " \0")) != NULL) {
						memset(buff2, 0, BUF_SIZE);
						strcpy(buff2, "ERROR");
					}
				}
			}

			// rename : rename from and to. rename -> RNFR & RNTO
			else if (!strcmp(ch, "rename")) {
				strcpy(buff2, "RNFR & RNTO ");

				// no argument
				if ((ch = strtok(NULL, " \0")) == NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
				// argument(from) exists.
				else {
					strcat(buff2, ch);
					strcat(buff2, " ");
				
					// argument(to) does not exist.
					if ((ch = strtok(NULL, " \0")) == NULL) {
						memset(buff2, 0, BUF_SIZE);
						strcpy(buff2, "ERROR");
					}
					// argument(to) exists.
					else {
						strcat(buff2, ch);

						// more argument exists : wrong input
						if ((ch = strtok(NULL, " \0")) != NULL) {
							memset(buff2, 0, BUF_SIZE);
							strcpy(buff2, "ERROR");
						} 
					}
				}
			}

			// mkdir : Make Directory. mkdir -> MKD
			else if (!strcmp(ch, "mkdir")) {
				strcpy(buff2, "MKD");

				// no argument
				if ((ch = strtok(NULL, " \0")) == NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
				// argument exists.
				else {
					while (ch != NULL) {
						strcat(buff2, " ");
						strcat(buff2, ch);
						ch = strtok(NULL, " \0");
					}
				}
			}

			// rmdir : Remove Directory. rmdir -> RMD
			else if (!strcmp(ch, "rmdir")) {
				strcpy(buff2, "RMD");
			
				// no argument
				if ((ch = strtok(NULL, " \0")) == NULL) {
					memset(buff2, 0, BUF_SIZE);
					strcpy(buff2, "ERROR");
				}
				// argument exists.
				else {
					while (ch != NULL) {
						strcat(buff2, " ");
						strcat(buff2, ch);
						ch = strtok(NULL, " \0");
					}
				}
			}

			// wrong input : exception
			else
				strcpy(buff2, "ERROR");
		}

		// send FTP command and any arguments if necessary
		write(sockfd, buff2, BUF_SIZE);

		// get the result from the server
		read(sockfd, rcv_buff, BUF_SIZE);

		// implement according to the result
		// QUIT
		if (!strcmp(rcv_buff, "QUIT_SUCCESS")) {
			exit(1);
		}

		// CWD, DELE, RNFR & RNTO, MKD, RMD, and ERROR
		else if (!strcmp(rcv_buff, "CWD_SUCCESS") || !strcmp(rcv_buff, "DELE_SUCCESS") || !strcmp(rcv_buff, "RNFR & RNTO_SUCCESS") || !strcmp(rcv_buff, "MKD_SUCCESS") || !strcmp(rcv_buff, "RMD_SUCCESS") || !strcmp(rcv_buff, "ERROR"))
			continue;
		// NLST, LIST, PWD
		else
			write(STDOUT_FILENO, rcv_buff, BUF_SIZE);	
	}	

	close(sockfd);
}

//////////////////////////////////////////////////////////////////
// sig_handler							//
// ============================================================ //
// Input : int -> signo						//
// Output : Nothing						//
//								//
// Purpose : process for SIGINT					//
//////////////////////////////////////////////////////////////////
void sig_handler(int signo) {
	memset(buff2, 0, BUF_SIZE);
	strcpy(buff2, "QUIT");
}

void sigalrm_handler(int signo) {
	time_t now;
	time(&now);
	printf("SIGALRM received! Current client time: %s", ctime(&now));

	// set next alarm(in 10 secs)
	alarm(10);
}

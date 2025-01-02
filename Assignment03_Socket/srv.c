//////////////////////////////////////////////////////////////////
// File name : srv.c						//
// Date : 2020.06.05						//
// OS : Ubuntu 16.04 64bits					//
// Author : Jeon Seung Hyeon					//
// Student ID : 2018202038					//
// ------------------------------------------------------------	//
// Title : System Programming Assignment #3			//
// Description : server						//
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
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUF_SIZE 1024
#define INFO_SIZE 32

int process_info[INFO_SIZE][2];		// store the information of processes orderly

void sh_chld(int);			// signal handler for SIGCHLD

//////////////////////////////////////////////////////////////////
// RWX								//
// ============================================================ //
// Input : mode_t -> file_mode,					//
//		the file mode					//
// 	   char -> buf_RWX[]					//
//		stored with RWX					//
//								//
// Output : nothing						//
//								//
// Purpose : Earn RWX						//
//////////////////////////////////////////////////////////////////
void RWX(mode_t file_mode, char buf_RWX[]) {

	if (S_ISDIR(file_mode))			// Directory
		strcpy(buf_RWX, "d");
	else					// Regular File
		strcpy(buf_RWX, "-");
	
	if (file_mode & S_IRUSR)		// User Read OK
		strcat(buf_RWX, "r");
	else					// User Read non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IWUSR)		// User Write OK
		strcat(buf_RWX, "w");
	else					// User Write non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IXUSR)		// User Execute OK
		strcat(buf_RWX, "x");
	else					// User Execute non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IRGRP)		// Group Read OK
		strcat(buf_RWX, "r");
	else					// Group Read non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IWGRP)		// Group Write OK
		strcat(buf_RWX, "w");
	else					// Group Write non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IXGRP)		// Group Execute OK
		strcat(buf_RWX, "x");
	else					// Group Execute non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IROTH)		// Other Read OK
		strcat(buf_RWX, "r");
	else					// Other Read non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IWOTH)		// Other Write OK
		strcat(buf_RWX, "w");
	else					// Other Write non-OK
		strcat(buf_RWX, "-");

	if (file_mode & S_IXOTH)		// Other Execute OK
		strcat(buf_RWX, "x");
	else					// Other Execute non-OK
		strcat(buf_RWX, "-");
}

//////////////////////////////////////////////////////////////////
// FILELIST							//
// ============================================================ //
// Input : char* -> ch,						//
//		FTP command (NLST or LIST)			//
//	   char -> result_buff[]				//
//		result according to the commands		//
//								//
// Output : int -1 fail						//
//		 0 success					//
//								//
// Purpose : Implement command					//
//////////////////////////////////////////////////////////////////
int FILELIST(char* ch, char result_buff[]) {
	DIR * dir = opendir(".");
	struct dirent * dp;
	struct stat buf;
	struct passwd *my_passwd;
	struct group *my_group;
	struct tm *timeinfo;
	char buf_st_nlink[32];
	char buf_st_size[32];
	char buf_time[64];
	char buf_RWX[32];
	int cnt = 0;

	// NLST (without option)
	if (!strcmp(ch, "NLST")) {
		while ((dp = readdir(dir)) != 0) {
			// hidden file could not be shown.
			if (dp->d_name[0] == '.')
				continue;
			else {
				// first time
				if (cnt == 0) {
					strcpy(result_buff, dp->d_name);	// file name
					strcat(result_buff, " ");
					cnt++;
				}
				// not first time
				else {
					strcat(result_buff, dp->d_name);	// file name
					strcat(result_buff, " ");
				}
			}		
		}
		strcat(result_buff, "\n");
		return 0;
	}

	// LIST
	else if (!strcmp(ch, "LIST")) {
		while ((dp = readdir(dir)) != 0) {
			memset(buf_st_nlink, 0, sizeof(buf_st_nlink));
			memset(buf_st_size, 0, sizeof(buf_st_size));
			memset(buf_time, 0, sizeof(buf_time));
			memset(buf_RWX, 0, sizeof(buf_RWX));
			
			if (lstat(dp->d_name, &buf) < 0)
				continue;
			
			// first time
			if (cnt == 0) {
				my_passwd = getpwuid(buf.st_uid);
				my_group = getgrgid(buf.st_gid);

				timeinfo = localtime(&(buf.st_mtime));
				strftime(buf_time, sizeof(buf_time), "%m/%d %H:%M", timeinfo);

				RWX(buf.st_mode, buf_RWX);
					
				sprintf(buf_st_nlink, "%ld", buf.st_nlink);
				sprintf(buf_st_size, "%ld", buf.st_size);
				
				strcpy(result_buff, buf_RWX);			// RWX
				strcat(result_buff, " ");
				strcat(result_buff, buf_st_nlink);		// the number of links
				strcat(result_buff, " ");
				strcat(result_buff, my_passwd->pw_name);	// User name
				strcat(result_buff, " ");
				strcat(result_buff, my_group->gr_name);		// Group name
				strcat(result_buff, " ");
				strcat(result_buff, buf_st_size);		// size
				strcat(result_buff, " ");
				strcat(result_buff, buf_time);			// time
				strcat(result_buff, " ");
				strcat(result_buff, dp->d_name);		// file name
				strcat(result_buff, "\n");
				cnt++;
			}
			// not first time
			else {
				my_passwd = getpwuid(buf.st_uid);
				my_group = getgrgid(buf.st_gid);
					
				timeinfo = localtime(&(buf.st_mtime));
				strftime(buf_time, sizeof(buf_time), "%m/%d %H:%M", timeinfo);
					
				RWX(buf.st_mode, buf_RWX);
					
				sprintf(buf_st_nlink, "%ld", buf.st_nlink);
				sprintf(buf_st_size, "%ld", buf.st_size);

				strcat(result_buff, buf_RWX);			// RWX
				strcat(result_buff, " ");
				strcat(result_buff, buf_st_nlink);		// the number of links
				strcat(result_buff, " ");
				strcat(result_buff, my_passwd->pw_name);	// User name
				strcat(result_buff, " ");
				strcat(result_buff, my_group->gr_name);		// Group name
				strcat(result_buff, " ");
				strcat(result_buff, buf_st_size);		// size
				strcat(result_buff, " ");
				strcat(result_buff, buf_time);			// time
				strcat(result_buff, " ");
				strcat(result_buff, dp->d_name);		// file name
				strcat(result_buff, "\n");
			}
		}		
		return 0;
	}

	else
		return -1;
}

// handler function for SIGALRM
void sigalrm_handler(int signo)
{
	time_t now;
	time(&now);
	printf("SIGALRM received! Current server time: %s", ctime(&now));

	alarm(5);
}

//////////////////////////////////////////////////////////////////
// main								//
// ============================================================ //
// Input : int -> argc,						//
//	   char** -> argv					//
//								//
// Output : practically nothing					//
//								//
// Purpose : Server						//
//////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	char buff1[BUF_SIZE];
	char buff2[BUF_SIZE];
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;
	int len;
	int child_cnt = 0;			// the number of child processes
	char* ch1;
	char* ch2;

	// First, Applying signal handler (sh_chld) for SIGCHLD
	signal(SIGCHLD, sh_chld);

	// Second, Applying signal hander for SIGALRM
	signal(SIGALRM, sigalrm_handler);

	// Third, set alarm timer(SIGALRM in 5 secs)
	alarm(5);

	// open socket, bind and listen
	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	// initialization for the information of the process
	for (int i = 0; i < INFO_SIZE; i++) {
		for (int j = 0; j < 2; j++)
			process_info[i][j] = 0;
	}

	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

	listen(server_fd, 5);

	while (1) {
		pid_t pid;
		len = sizeof(client_addr);

		while ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len)) >= 0) {	
			
			if (client_fd < 0) {
				write(STDOUT_FILENO, "Server : Accept Fail.\n", sizeof("Server : Accept Fail.\n"));
				return 0;
			}
			else {
				printf("==========client info==========\n");
				printf("client IP : %s\n\n", inet_ntoa(client_addr.sin_addr));
				printf("client port : %d\n", client_addr.sin_port);
				printf("===============================\n");
			}

			// Fork
			if ((pid = fork()) < 0) {
				printf("Server : Fork Fail.\n");
				return 0;
			}
			else if (pid == 0) {
				close(server_fd);

				process_info[child_cnt][0] = getpid();
				process_info[child_cnt][1] = client_addr.sin_port;
				child_cnt++;
				
				printf("Child Process ID : %d\n", getpid());
				printf("PID : %d, Port : %d\n", process_info[child_cnt-1][0], process_info[child_cnt-1][1]);

				memset(buff1, 0, BUF_SIZE);
				// memset(buff2, 0, BUF_SIZE);
				while (read(client_fd, buff1, BUF_SIZE) > 0) {
					write (STDOUT_FILENO, "> ", sizeof("> "));
					memset(buff2, 0, BUF_SIZE);

					ch1 = strtok(buff1, " \0");
					
					// PWD
					if (!strcmp(ch1, "PWD")) {
						printf("%s\t[%d]\n", ch1, getpid());

						getcwd(buff2, BUF_SIZE);
						strcat(buff2, "\n");
						write(client_fd, buff2, BUF_SIZE);
					}

					// NLST
					else if (!strcmp(ch1, "NLST")) {
						printf("%s\t[%d]\n", ch1, getpid());
						
						if (FILELIST(ch1, buff2) == 0)
							write(client_fd, buff2, BUF_SIZE);
					}

					// LIST
					else if (!strcmp(ch1, "LIST")) {
						printf("%s\t[%d]\n", ch1, getpid());
			
						if (FILELIST(ch1, buff2) == 0)
							write(client_fd, buff2, BUF_SIZE);
					}			

					// QUIT
					else if (!strcmp(ch1, "QUIT")) {
						printf("Client(%d)'s Release\n\n", getpid());
						strcpy(buff2, "QUIT_SUCCESS");
						
						for (int i = 0; i < INFO_SIZE; i++) {
							if (process_info[i][0] == getpid()) {
								process_info[i][0] = 0;
								process_info[i][1] = 0;
							}
							else
								continue;
						}
							
						write(client_fd, buff2, BUF_SIZE);
						close(client_fd);
						break;
					}

					// CWD
					else if (!strcmp(ch1, "CWD")) {
						printf("%s\t[%d]\n", ch1, getpid());

						ch1 = strtok(NULL, " \0");
						if (chdir(ch1) == 0) {
							strcpy(buff2, "CWD_SUCCESS");
							write(client_fd, buff2, BUF_SIZE);
						}	
					}

					// DELE
					else if (!strcmp(ch1, "DELE")) {
						printf("%s\t[%d]\n", ch1, getpid());

						ch1 = strtok(NULL, " \0");
						if (unlink(ch1) == 0) {
							strcpy(buff2, "DELE_SUCCESS");
							write(client_fd, buff2, BUF_SIZE);
						}
					}

					// RNFR & RNTO
					else if (!strcmp(ch1, "RNFR")) {
						strcpy(buff2, ch1);		// buff2 : "RNFR"
						strcat(buff2, " ");		// buff2 : "RNFR "
						ch1 = strtok(NULL, " \0");		
						strcat(buff2, ch1);		// buff2 : "RNFR &"
						strcat(buff2, " ");		// buff2 : "RNFR & "
						ch1 = strtok(NULL, " \0");
						strcat(buff2, ch1);		// buff2 : "RNFR & RNTO"
			
						printf("%s\t[%d]\n", buff2, getpid());

						ch1 = strtok(NULL, " \0");		// ch2 is an old name
						ch2 = ch1;
						ch1 = strtok(NULL, " \0");		// ch1 is a new name
			
						if (rename(ch2, ch1) == 0) {
							strcpy(buff2, "RNFR & RNTO_SUCCESS");
							write(client_fd, buff2, BUF_SIZE);
						}
					}

					// MKD
					else if (!strcmp(ch1, "MKD")) {
						printf("%s\t[%d]\n", ch1, getpid());

						while (ch1 != NULL) {
							ch1 = strtok(NULL, " \0");
					
							if (mkdir(ch1, 0755) == 0)
						strcpy(buff2, "MKD_SUCCESS");
						}
						write(client_fd, buff2, BUF_SIZE);
					}	

					// RMD
					else if (!strcmp(ch1, "RMD")) {
						printf("%s\t[%d]\n", ch1, getpid());

						while (ch1 != NULL) {
							ch1 = strtok(NULL, " \0");

							if (rmdir(ch1) == 0)
								strcpy(buff2, "RMD_SUCCESS");
						}
						write(client_fd, buff2, BUF_SIZE);
					}

					// ERROR
					else {
						printf("%s\t[%d]\n", ch1, getpid());
						strcpy(buff2, "ERROR");
						write(client_fd, buff2, BUF_SIZE);
					}		
				}
			
				close(client_fd);
				exit(0);
			}
			
			else
				close(client_fd);
		}
	}

	close(server_fd);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// sh_chld								//
// ==================================================================== //
// Input : int -> signum						//
//		; SIGCHLD						//
// Output : Nothing							//
// Purpose : Notify status of the child process was changed.		//
//////////////////////////////////////////////////////////////////////////
void sh_chld (int signum) {
	wait(NULL);
}

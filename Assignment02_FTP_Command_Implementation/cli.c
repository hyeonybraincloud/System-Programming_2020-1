//////////////////////////////////////////////////////////////////////////
// File Name : cli.c													//
// Date : 2020/05/08													//
// OS : Ubuntu 16.04 64 bits											//
// Author : OOOOOOOOOOOOOO												//
// Student ID : 0000000000												//
// -------------------------------------------------------------------- //
// Title : System Programming											//
// Description : Implement FTP server1									//
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

//////////////////////////////////////////////////////////////////////////
// main																	//
// ==================================================================== //
// Input : int -> argc													//
//			char* -> argv[]												//
// Output : Nothing														//
// Purpose : To get commands from the Client							//
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
	
	int aflag=0, lflag=0;				// flags for options -a, -l, and -al
	int c;						// what is chosen
	char buf[128];					// buffer
	struct stat buffer;				// for File/Directory Name
	opterr = 0;

	memset(buf, 0, sizeof(buf));

	// pwd -> PWD; Print Current Working Directory
	if (!strcmp("pwd", argv[1])) {
		if (argc-1 != 1)			// when additional argument is written
			strcpy(buf, "ERROR");
			
		else					// convert to PWD
			strcpy(buf, "PWD");
	}
	
	// mkdir -> MKD; Make Directory
	else if (!strcmp("mkdir", argv[1])) {
		if (argc-1 != 2)				// when additional argument is written
			strcpy(buf, "ERROR");
		else {						// convert to MKD
			strcpy(buf, "MKD");
			for (int i = 2; i < argc; i++) {	// to get the new directory name
				strcat(buf, " ");
				strcat(buf, argv[i]);
			}
		}
	}

	// rmdir -> RMD; Remove Directory
	else if (!strcmp("rmdir", argv[1])) {
		if (argc-1 != 2)				// when additional argument is written
			strcpy(buf, "ERROR");
		else {						// convert to RMD
			strcpy(buf, "RMD");
			for (int i = 2; i < argc; i++) {	// to get the directory name to be removed
				strcat(buf, " ");
				strcat(buf, argv[i]);
			}
		}
	}
	
	// delete -> DELE; Remove File
	else if (!strcmp("delete", argv[1])) {
		if (argc-1 != 2)				// when additional argument is written
			strcpy(buf, "ERROR");
		else { 						// convert to DELE
			strcpy(buf, "DELE");
			for (int i = 2; i < argc; i++) {	// to get the file name to be removed
				strcat(buf, " ");
				strcat(buf, argv[i]);
			}
		}
	}

	// rename -> RNFR & RNTO; Remove from and to
	else if (!strcmp("rename", argv[1])) {
		if (argc-1 != 3)		// when additional argument is written or the item is not sufficient
			strcpy(buf, "ERROR");

		else {				// convert to RNFR & RNTO
			strcpy(buf, "RNFR & RNTO");
			for (int i = 2; i < argc; i++) {// to get the file name to be renamed and the modified name
				strcat(buf, " ");
				strcat(buf, argv[i]);
			}
		}
	}
	
	// cd ; Change Working Directory
	else if (!strcmp("cd", argv[1])) {
		if (argc-1 != 2)				// when additional argument is written
			strcpy(buf, "ERROR");
		
		else {
			if (!strcmp(argv[2], "..")) {		// cd ..; Change Directory to ..
				strcpy(buf, "CDUP");		// convert to CDUP
				for (int i = 2; i < argc; i++) {	
					strcat(buf, " ");
					strcat(buf, argv[i]);
				}
			}
			else {
				strcpy(buf, "CD");//  cd [Directory]; Change Directory to [Directory], convert to CD
				for (int i = 2; i < argc; i++) {	
					strcat(buf, " ");
					strcat(buf, argv[i]);
				}
			}
		}
	}

	// dir -> LIST; Directory Entry List
	else if (!strcmp("dir", argv[1])) {
		if (argc-1 >= 3)		// when additional argument is written
			strcpy(buf, "ERROR");
		else {
			if (lstat(argv[2], &buffer) < 0)	// when access to the argv[2] is not permitted
				strcpy(buf, "ERROR");
			else {
				if (S_ISDIR(buffer.st_mode)) {	// Is argv[2] a Directory?
					strcpy(buf, "LIST ");	// convert to LIST
					strcat(buf, argv[2]);
				}
				else				// When the argv[2] is not directory
					strcpy(buf, "ERROR");
			}
		}
			
	}

	// quit -> QUIT; Terminate the command connection
	else if (!strcmp("quit", argv[1])) {
		if (argc-1>=2)					// when additional argument is written
			strcpy(buf, "ERROR");
		else						// convert to QUIT
			strcpy(buf, "QUIT");
	}

	// ls -> NLST; Name list
	else if (!strcmp("ls", argv[1])) {
		if (argc-1>=4)					// when too much arguments are written
			strcpy(buf, "ERROR");
		else if (argc-1==1)				// No option
			strcpy(buf, "NLST . N");	
	// N means None-option. Originally, the command does not have option. However, for convenience, N is passed.

		else {						// option exists
			while ((c=getopt(argc, argv, "al")) != -1) {
				switch(c) {
					case 'a':		// -a or -al
						aflag = 1;
						break;
					case 'l':		// -l or -al
						lflag = 1;
						break;
					default :		// Something is written on the option plot, which means error.
						aflag = 0;
						lflag = 0;
				}
			}
		
			if (argc-1==2) {				// ls [option]
				if (aflag == 1 || lflag == 1) {
				strcpy(buf, "NLST . ");			// "." means the current working directory. It is for convenience.
					strcat(buf, argv[1]);
				}
				else
					strcpy(buf, "ERROR");
			}

			else {						// ls [argument] [option]
				if (lstat(argv[3], &buffer) < 0)	// when access to the argv[3] is not permitted		
					strcpy(buf, "ERROR");
				else {
					if (aflag == 1 && lflag == 1) {		// the option -al
						if (S_ISDIR(buffer.st_mode)) {	// Is argv[3] Directory?
							strcpy(buf, "NLST ");
							strcat(buf, argv[3]);
							strcat(buf, " ");
							strcat(buf, argv[1]);
						}
						else				// when argv[3] is not directory
							strcpy(buf, "ERROR");
					}
					else if (aflag == 1 && lflag == 0) {		// the option -a
						if (S_ISDIR(buffer.st_mode)) {		// Is argv[3] Directory?
							strcpy(buf, "NLST ");
							strcat(buf, argv[3]);
							strcat(buf, " ");
							strcat(buf, argv[1]);
						}
						else					// when argv[3] is not directory
							strcpy(buf, "ERROR");
					}
					else if (aflag == 0 && lflag == 1) {		// the option -l
						if (S_ISDIR(buffer.st_mode) || S_ISREG(buffer.st_mode)) {	// Is argv[3] Directory or Regular file?
							strcpy(buf, "NLST ");
							strcat(buf, argv[3]);
							strcat(buf, " ");
							strcat(buf, argv[1]);
						}
						else					// when argv[3] is neither directory nor regular file
							strcpy(buf, "ERROR");
					}
					else					// Something is written on the option plot, which means error.
						strcpy(buf, "ERROR");
				}		
			}
		}
	}

	else						// Absolute Wrong Input
		strcpy(buf, "ERROR");

	write(1, buf, sizeof(buf));			// Write system call

	return 0;
}

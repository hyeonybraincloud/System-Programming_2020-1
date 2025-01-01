//////////////////////////////////////////////////////////////////////////////////////////
// File Name : srv.c																	//
// Date : 2020/05/08																	//
// OS : Ubuntu 16.04 64 bits															//
// Author : OOOOOOOOOOOOOO																//
// Student ID : 0000000000																//
// ------------------------------------------------------------------------------------ //
// Title : System Programming															//
// Description : Implement FTP Server1													//
//////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////////////////
// RWX																					//
// ==================================================================================== //
// Input : mode_t -> file_mode															//
//				To recognize what type is												//
// Output : Nothing																		//
// Purpose : Access Authority represented with number is converted to text				//
//////////////////////////////////////////////////////////////////////////////////////////

void RWX(mode_t file_mode) {

	if (S_ISDIR(file_mode))			// Directory
		printf("d");
	else					// Regular File
		printf("-");
	
	if (file_mode & S_IRUSR)		// User Read OK
		printf("r");
	else					// User Read non-OK
		printf("-");

	if (file_mode & S_IWUSR)		// User Write OK
		printf("w");
	else					// User Write non-OK
		printf("-");

	if (file_mode & S_IXUSR)		// User Execute OK
		printf("x");
	else					// User Execute non-OK
		printf("-");

	if (file_mode & S_IRGRP)		// Group Read OK
		printf("r");
	else					// Group Read non-OK
		printf("-");

	if (file_mode & S_IWGRP)		// Group Write OK
		printf("w");
	else					// Group Write non-OK
		printf("-");

	if (file_mode & S_IXGRP)		// Group Execute OK
		printf("x");
	else					// Group Execute non-OK
		printf("-");

	if (file_mode & S_IROTH)		// Other Read OK
		printf("r");
	else					// Other Read non-OK
		printf("-");

	if (file_mode & S_IWOTH)		// Other Write OK
		printf("w");
	else					// Other Write non-OK
		printf("-");

	if (file_mode & S_IXOTH)		// Other Execute OK
		printf("x ");
	else					// Other Execute non-OK
		printf("- ");
}

//////////////////////////////////////////////////////////////////////////////////////////
// NLST											//
// ==================================================================================== //
// Input : char* -> Name[]; File/Directory Name						//
//			char* -> filepath; pathname					//
//			char* -> ch; option						//
// Output : Nothing									//
// Purpose : NLST command implementation						//
//////////////////////////////////////////////////////////////////////////////////////////

void NLST(char *Name[], char* filepath, char *ch) {
	DIR * dir = opendir(filepath);				// open the directory "filepath"
	struct dirent * dp;
	struct stat buf;
	struct passwd *my_passwd;				// for owner
	struct group *my_group;					// for group
	struct tm *timeinfo;					// time information
	int cnt = 0, show = 0;					// index for the Name array, index for representing with 5 contents on one row
	char * temp_str;					// for bubble-sorting
	char buffer[64];					// buffer
	
	if (dir == NULL) {					// when the "filepath" is regular file
		lstat(filepath, &buf);				// associated with the "filepath"
		
		my_passwd = getpwuid(buf.st_uid);			// owner
		my_group = getgrgid(buf.st_gid);			// group

		timeinfo = localtime(&(buf.st_mtime));			// time information
		strftime(buffer, sizeof(buffer), "%m/%d %H:%M", timeinfo);	// Month/Day, Hour:Minute
		
		RWX(buf.st_mode);							// access authority
		// the number of links, owner name, group name, the file size, the final modification time, and the file name
		printf("%ld %s %s %ld %s %s\n", buf.st_nlink, my_passwd->pw_name, my_group->gr_name, buf.st_size, buffer, filepath);
	}

	// when the "filepath" is directory
	else {
		while((dp=readdir(dir)) != 0) {					// read directory
			Name[cnt] = (char*)malloc(sizeof(dp->d_name)+1);	// Dynamic Allocation
			strcpy(Name[cnt], dp->d_name);
			cnt++;
		}
		
		// bubble sorting in ascending order
		for (int step=0; step<cnt-1; step++) {
			for (int i=0; i < cnt-1-step; i++) {
				if (strcmp(Name[i], Name[i+1]) > 0) {
					temp_str = Name[i];
					Name[i] = Name[i+1];
					Name[i+1] = temp_str;
				}
			}
		}

		if (!strcmp(ch, "-a")) {							// option -a
			for (int i=0; i<cnt; i++) {
				if (lstat(Name[i], &buf) < 0)					// associated with the Name
					continue;

				if (S_ISREG(buf.st_mode))					// Regular file?
					printf("%s ", Name[i]);
				else if (S_ISDIR(buf.st_mode))					// Directory?
					printf("%s/ ", Name[i]);				// Add '/'
	
				if (i%5==4 || Name[i+1] == NULL)
					printf("\n");
			}
		}

		else if (!strcmp(ch, "-l")) {							// option -l
			for (int i=0; i<cnt; i++) {
				if (lstat(Name[i], &buf) < 0)					// associated with the Name
					continue;
				if (Name[i][0] == '.')						// the hidden file is not included.
					continue;

				my_passwd = getpwuid(buf.st_uid);				// owner
				my_group = getgrgid(buf.st_gid);				// group
			
				timeinfo = localtime(&(buf.st_mtime));				// time information
				strftime(buffer, sizeof(buffer), "%m/%d %H:%M", timeinfo);	// Month/Day, Hour:Minute
			
				RWX(buf.st_mode);								// access authority

				// the number of links, owner name, group name, the file size, and the final modification time
				printf("%ld %s %s %ld %s ", buf.st_nlink, my_passwd->pw_name, my_group->gr_name, buf.st_size, buffer);			

				if (S_ISREG(buf.st_mode))						// Regular file?
					printf("%s\n", Name[i]);
				else if (S_ISDIR(buf.st_mode))						// Directory?
					printf("%s/\n", Name[i]);					// Add '/'
			}
		}

		else if (!strcmp(ch, "-al")) {							// option -al
			for (int i=0; i<cnt; i++) {
				if (lstat(Name[i], &buf) < 0)					// associated with the Name
					continue;

				my_passwd = getpwuid(buf.st_uid);				// owner
				my_group = getgrgid(buf.st_gid);				// group
			
				timeinfo = localtime(&(buf.st_mtime));				// time information
				strftime(buffer, sizeof(buffer), "%m/%d %H:%M", timeinfo);	// Month/Day, Hour:Minute

				RWX(buf.st_mode);								// access authority

				// the number of links, owner name, group name, the file size, and the final modification time
				printf("%ld %s %s %ld %s ", buf.st_nlink, my_passwd->pw_name, my_group->gr_name, buf.st_size, buffer);

				if (S_ISREG(buf.st_mode))					// Regular file?
					printf("%s\n", Name[i]);
				else if (S_ISDIR(buf.st_mode))					// Directory?
					printf("%s/\n", Name[i]);				// Add '/'
			}
		}

		else {										// Non-option
			for (int i=0; i<cnt; i++) {
				if (lstat(Name[i], &buf) <0)					// associated with the Name
					continue;
				if (Name[i][0] == '.')						// the hidden file is not included.
					continue;

				if (S_ISREG(buf.st_mode))					// Regular file?
					printf("%s ", Name[i]);
				else if (S_ISDIR(buf.st_mode))					// Directory?
					printf("%s/ ", Name[i]);				// Add '/'
			
				if (show%5==4 || Name[i+1] == NULL)
					printf("\n");

				show++;
			}
		}
	
		for (int i=0; i < cnt; i++)							// free the dynamic allocations
			free(Name[i]);
		
		closedir(dir);									// close the directory
	}
	
}

//////////////////////////////////////////////////////////////////////////////////////////
// main											//
// ==================================================================================== //
// Input : int -> argc									//
//			char* -> argv[]							//
// Output : Nothing									//
// Purpose : FTP commands implementation						//
//////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

	char getStr[128];			// get String
	char buf[128];				// buffer
	char FTPwords[128];			// FTP commands to be shown on the display
	char *ch;				// to read
	char *Name[128];			// file/Directory name lists
	
	// initialization
	memset(getStr, 0, sizeof(getStr));
	memset(buf, 0, sizeof(buf));
	memset(FTPwords, 0, sizeof(FTPwords));
	memset(Name, 0, sizeof(Name));

	read(0, getStr, sizeof(getStr));	// read

	ch = strtok(getStr, " \0");

	// PWD; Print Current Working Directory
	if (!strcmp(ch, "PWD")) {
		printf("%s\n", ch);
		printf("%s\n", getcwd(buf, sizeof(buf)));
	}

	// MKD; Make Directory
	if (!strcmp(ch, "MKD")) {	
		strcpy(FTPwords, ch);
		while (ch != NULL) {
			ch = strtok(NULL, " \0");
			if (mkdir(ch, 0755) == 0) {
				strcat(FTPwords, " ");
				strcat(FTPwords, ch);
			}
		}
		
		if (strcmp(FTPwords, "MKD"))
			printf("%s\n", FTPwords);
	}
	
	// RMD; Remove Directory
	else if (!strcmp(ch, "RMD")) {
		strcpy(FTPwords, ch);
		while (ch != NULL) {
			ch = strtok(NULL, " \0");
			if (rmdir(ch) == 0) {
				strcat(FTPwords, " ");
				strcat(FTPwords, ch);
			}
		}
		
		if (strcmp(FTPwords, "RMD"))
			printf("%s\n", FTPwords);
	}

	// DELE; Remove file
	else if (!strcmp(ch, "DELE")) {
		strcpy(FTPwords, ch);
		while (ch != NULL) {
			ch = strtok(NULL, " \0");
			if (unlink(ch) == 0) {
				strcat(FTPwords, " ");
				strcat(FTPwords, ch);
			}
		}

		if (strcmp(FTPwords, "DELE"))
			printf("%s\n", FTPwords);
	}

	// RNFR & RNTO; rename from and to
	else if (!strcmp(ch, "RNFR")) {
		strcpy(FTPwords, ch);
		while (1) {
			ch = strtok(NULL, " \0");
			strcat(FTPwords, " ");
			strcat(FTPwords, ch);
			if (!strcmp(ch, "RNTO"))
				break;
		}
		ch = strtok(NULL, " \0");
		strcpy(buf, ch);
		ch = strtok(NULL, " \0");
		
		if (rename(buf, ch) == 0) {
			strcat(FTPwords, " ");
			strcat(FTPwords, buf);
			strcat(FTPwords, " ");
			strcat(FTPwords, ch);
			printf("%s\n", FTPwords);
		}		
	}

	// CDUP or CD; Change working directory to .. or [..]
	else if (!strcmp(ch, "CDUP") || !strcmp(ch, "CD")) {
		strcpy(FTPwords, ch);
		ch = strtok(NULL, " \0");
		if (chdir(ch) == 0) {
			strcat(FTPwords, " ");
			strcat(FTPwords, ch);
			printf("%s\n", FTPwords);
			printf("%s\n", getcwd(buf, sizeof(buf)));
		}
	}

	// LIST; Directory Entry List
	else if (!strcmp(ch, "LIST")) {
		strcpy(FTPwords, ch);
		strcat(FTPwords, " ");

		ch = strtok(NULL, " \0");
		strcpy(buf, ch);
		strcpy(ch, "-al");
		strcat(FTPwords, buf);

		printf("%s\n", FTPwords);
		NLST(Name, buf, ch);
	}

	// QUIT; terminate the command connection
	else if (!strcmp(ch, "QUIT")) {
		strcpy(FTPwords, ch);
		
		printf("%s\n", FTPwords);
		printf("QUIT SUCCESS\n");
		exit(0);
	}

	// NLST; Name list
	else if (!strcmp(ch, "NLST")) {
		strcpy(FTPwords, ch);
		strcat(FTPwords, " ");
		
		ch = strtok(NULL, " \0");
		strcpy(buf, ch);

		// If the directory to be revealed is the current, it is not included in the FTPwords.
		if (strcmp(ch, ".")) {
			strcat(FTPwords, ch);
			strcat(FTPwords, " ");
		}
		
		ch = strtok(NULL, " \0");
		
		// N, which is non-option, is not included in the FTPwords. Because it is made just for convenience.
		if (strcmp(ch, "N"))
			strcat(FTPwords, ch);

		printf("%s\n", FTPwords);
		NLST(Name, buf, ch);		
	}

	// ERROR
	else
		return 0;

	return 0;
}

/*****************************************************************************
 *  File: cli.c
 *  Description: Simple FTP client with user command -> FTP command conversion,
 *               connect to server, receive replies, etc.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>

#define MAX_BUF 1024

int g_sockfd = -1; // ���� ���� fd, Ctrl+C �� ó����

void sigint_handler(int signo)
{
    // Ctrl+C �� QUIT ����
    if (g_sockfd >= 0) {
        write(g_sockfd, "QUIT\r\n", 6);
    }
    // ���� ����
    exit(0);
}

// ����� �Է� -> FTP ��� ó��
void ftp_client_loop(int sockfd)
{
    char recv_buf[MAX_BUF], send_buf[MAX_BUF];
    while (1) {
        // ������Ʈ
        write(STDOUT_FILENO, "ftp> ", 5);
        // ��ɾ� �Է�
        memset(send_buf, 0, sizeof(send_buf));
        int n = read(STDIN_FILENO, send_buf, MAX_BUF - 1);
        if (n <= 0) {
            break;
        }
        // ���� ����
        if (send_buf[n - 1] == '\n')
            send_buf[n - 1] = '\0';

        // ����ڰ� quit, bye, exit
        if (!strcmp(send_buf, "quit") || !strcmp(send_buf, "bye") || !strcmp(send_buf, "exit")) {
            // ���� FTP �������ݷδ� "QUIT\r\n"
            char tmp[] = "QUIT\r\n";
            write(sockfd, tmp, strlen(tmp));
            // ���� ���� �ޱ�
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) {
                write(STDOUT_FILENO, recv_buf, r);
            }
            break;
        }
        // pwd -> "PWD\r\n"
        else if (!strcmp(send_buf, "pwd")) {
            char tmp[] = "PWD\r\n";
            write(sockfd, tmp, strlen(tmp));
            // recv
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) write(STDOUT_FILENO, recv_buf, r);
        }
        // cd XXX -> "CWD XXX\r\n"
        else if (!strncmp(send_buf, "cd ", 3)) {
            char arg[512];
            strcpy(arg, send_buf + 3);
            char tmp[600];
            snprintf(tmp, sizeof(tmp), "CWD %s\r\n", arg);
            write(sockfd, tmp, strlen(tmp));
            // recv
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) write(STDOUT_FILENO, recv_buf, r);
        }
        else if (!strcmp(send_buf, "ls")) {
            // ls -> NLST
            char tmp[] = "NLST\r\n";
            write(sockfd, tmp, strlen(tmp));
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) write(STDOUT_FILENO, recv_buf, r);
        }
        else if (!strncmp(send_buf, "mkdir ", 6)) {
            // mkdir <dirname> -> MKD <dirname>
            char tmp[MAX_BUF];
            snprintf(tmp, sizeof(tmp), "MKD %s\r\n", send_buf + 6);
            write(sockfd, tmp, strlen(tmp));
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) write(STDOUT_FILENO, recv_buf, r);
        }
        else if (!strncmp(send_buf, "rmdir ", 6)) {
            // rmdir <dirname> -> RMD <dirname>
            char tmp[MAX_BUF];
            snprintf(tmp, sizeof(tmp), "RMD %s\r\n", send_buf + 6);
            write(sockfd, tmp, strlen(tmp));
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) write(STDOUT_FILENO, recv_buf, r);
        }
        else if (!strncmp(send_buf, "get ", 4)) {
            // get <filename> -> RETR <filename>
            char filename[512];
            strcpy(filename, send_buf + 4);
            char tmp[600];
            snprintf(tmp, sizeof(tmp), "RETR %s\r\n", filename);
            write(sockfd, tmp, strlen(tmp));
            // ���� ������ ����
            char recv_buf[MAX_BUF];
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) {
                if (strncmp(recv_buf, "550", 3) == 0) {
                    write(STDOUT_FILENO, recv_buf, r); // ���� �޽��� ���
                }
                else {
                    // ������ ����
                    FILE* fp = fopen(filename, "w");
                    if (fp) {
                        fwrite(recv_buf, 1, r, fp);
                        fclose(fp);
                        write(STDOUT_FILENO, "File received and saved.\n", 25);
                    }
                }
            }
        }
        else if (!strncmp(send_buf, "put ", 4)) {
            // put <filename> -> STOR <filename>
            char filename[512];
            strcpy(filename, send_buf + 4);
            char tmp[600];
            snprintf(tmp, sizeof(tmp), "STOR %s\r\n", filename);
            write(sockfd, tmp, strlen(tmp));
            // ������ ���� ����
            FILE* fp = fopen(filename, "r");
            if (fp) {
                char file_buf[MAX_BUF];
                size_t read_bytes;
                while ((read_bytes = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
                    write(sockfd, file_buf, read_bytes);
                }
                fclose(fp);
                write(STDOUT_FILENO, "File uploaded.\n", 15);
            }
            else {
                write(STDOUT_FILENO, "Failed to open file for upload.\n", 32);
            }
        }
        else if (!strncmp(send_buf, "rename ", 7)) {
            // rename <oldname> <newname>
            char oldname[256], newname[256];
            if (sscanf(send_buf + 7, "%s %s", oldname, newname) != 2) {
                write(STDOUT_FILENO, "Usage: rename <oldname> <newname>\n", 35);
                continue;
            }
            char tmp[600];
            snprintf(tmp, sizeof(tmp), "RNFR %s\r\n", oldname);
            write(sockfd, tmp, strlen(tmp));
            // �����κ��� RNFR ���� Ȯ��
            char recv_buf[MAX_BUF];
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0 && strncmp(recv_buf, "350", 3) == 0) {
                snprintf(tmp, sizeof(tmp), "RNTO %s\r\n", newname);
                write(sockfd, tmp, strlen(tmp));
                memset(recv_buf, 0, sizeof(recv_buf));
                r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
                if (r > 0) write(STDOUT_FILENO, recv_buf, r);
            }
            else {
                write(STDOUT_FILENO, recv_buf, r);
            }
        }
        else {
            // ��Ÿ(���� ���� �� �� ���)
            // ��: ls, get, put, rename ���.
            // ���⼭�� �ܼ��� �״�� ����
            // ���� ���������� ls->NLST, get->RETR, put->STOR �� ��ȯ �ʿ�
            strcat(send_buf, "\r\n");
            write(sockfd, send_buf, strlen(send_buf));
            // ����
            memset(recv_buf, 0, sizeof(recv_buf));
            int r = read(sockfd, recv_buf, sizeof(recv_buf) - 1);
            if (r > 0) write(STDOUT_FILENO, recv_buf, r);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ServerIP> <ServerPort>\n", argv[0]);
        exit(1);
    }
    signal(SIGINT, sigint_handler);

    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    g_sockfd = sockfd;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // connect
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 0;
    }
    write(STDOUT_FILENO, "** Connected to server **\n", 27);

    // ������ ó���� ���� �� �ִ� ȯ��/�ź� �޽��� ����
    char buf[MAX_BUF];
    memset(buf, 0, MAX_BUF);
    int n = read(sockfd, buf, MAX_BUF - 1);
    if (n > 0) {
        if (!strncmp(buf, "REJECTION", 9)) {
            write(STDOUT_FILENO, "** Connection refused by server **\n", 35);
            close(sockfd);
            return 0;
        }
        // ACCEPTED or 220 ...
        write(STDOUT_FILENO, buf, n);
    }

    // ���� ���� ��� ����
    ftp_client_loop(sockfd);

    close(sockfd);
    return 0;
}

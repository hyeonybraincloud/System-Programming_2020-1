/*****************************************************************************
 *  File: srv.c
 *  Description: Concurrent FTP Server with logging, signals, user auth, etc.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/wait.h>

#define MAX_BUF  1024
#define IP_BUF   64
#define LOG_PATH "logfile"     // 로그 파일 이름
#define ACCESS_TXT "access.txt"
#define PASSWD_TXT "passwd"

 // FTP Reply Codes 예시
#define CODE_200 "200"
#define CODE_220 "220"
#define CODE_221 "221"
#define CODE_226 "226"
#define CODE_230 "230"
#define CODE_331 "331"
#define CODE_331_MSG "Password is required."
#define CODE_530 "530"
#define CODE_430 "430"
#define CODE_431 "431"
#define CODE_550 "550"
#define CODE_250 "250"
#define CODE_257 "257"
#define CODE_350 "350"
#define CODE_266 "266" // custom example

// --------------------------------------------------------
// 전역(부모)에서 사용할 함수 프로토타입
void sigchld_handler(int signo);
void sigint_handler(int signo);
void write_log(const char* ip, int port, const char* user,
    const char* comment, int show_time, int show_service_time, time_t start_time);

// --------------------------------------------------------
// 접속 허용 여부(access.txt)
int check_access_ip(const char* ip_str);

// --------------------------------------------------------
// 사용자 인증(passwd 파일)
int check_user_pass(const char* user, const char* pass);

// --------------------------------------------------------
// child 프로세스에서 각 클라이언트 세션 담당
void session_serve(int connfd, struct sockaddr_in cli_addr);

// --------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ServerPort>\n", argv[0]);
        exit(1);
    }

    // 서버 시작 시 로깅
    // 예: "Fri May 30 16:01:03 2017 Server is started"
    write_log(NULL, 0, NULL, "Server is started", 1, 0, 0);

    signal(SIGCHLD, sigchld_handler); // 좀비 프로세스 처리
    signal(SIGINT, sigint_handler);   // Ctrl + C 시 종료

    int listenfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(listenfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    // 메인 accept 루프
    while (1) {
        clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
        if (connfd < 0) {
            perror("accept");
            continue;
        }

        // 자식 생성
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(connfd);
            continue;
        }
        else if (pid == 0) {
            // 자식 프로세스
            close(listenfd);
            session_serve(connfd, cli_addr);
            close(connfd);
            exit(0);
        }
        // 부모
        close(connfd);
    }

    return 0;
}

// --------------------------------------------------------
// 자식 프로세스: 접속 IP 검사 -> ACCEPTED or REJECTION -> 인증 -> 명령 처리
void session_serve(int connfd, struct sockaddr_in cli_addr)
{
    char ip_str[IP_BUF];
    strcpy(ip_str, inet_ntoa(cli_addr.sin_addr));
    int cport = ntohs(cli_addr.sin_port);

    // 1) IP 허용 여부
    int allowed = check_access_ip(ip_str);
    if (!allowed) {
        // 로그: "Fri May 30 16:01:03 2017 [ip:port]  LOG_FAIL" (user는 아직 모름 -> "")
        write_log(ip_str, cport, "", "LOG_FAIL(IP)", 1, 0, 0);
        // REJECTION
        write(connfd, "REJECTION", 10);
        return;
    }
    // IP 허용
    // 220 welcome 메시지 전송
    // ex) "220 Server (myftp 1.0) ready\r\n"
    // 여기서는 간단히 ACCEPTED만.
    write(connfd, "ACCEPTED", 9);

    // 2) 사용자 인증 (USER/PASS 시퀀스)
    // 단순 구현 예: 3회 시도
    char user[64] = { 0 }, pass[64] = { 0 };
    int login_ok = 0, attempts = 0;

    static char oldname[MAX_BUF] = "";
    // read user -> read pass, etc. (실제 FTP는 "USER <user>", "PASS <pass>" 식)
    // 여기서는 간단히 한 번에 user pass 입력받는 예시로 대체 가능

    // => 만약 명령어 수준 구현하려면 Control loop에서 "USER"명령, PASS 명령 처리.

    // 여기서는 일단 "USER" + "PASS"를 한번에 받는 예시(단순화).
    // ex) "USER <username>\n" -> "331 Password required" -> "PASS <password>\n" ...
    // For brevity, 예시 로직:
    write(connfd, "220 Please input 'USER user' then 'PASS pass'", 44);

    // 실제 FTP 스펙으로 구현하려면 아래 control 명령 루프로 인증처리를 옮길 수 있음.
    // -- 여기선 간단 시연: 3회 반복
    while (attempts < 3 && !login_ok) {
        char buf[MAX_BUF] = { 0 };
        int n = read(connfd, buf, MAX_BUF);
        if (n <= 0) return; // 연결 끊김

        // 예: "USER seunghyeon"
        if (!strncmp(buf, "USER ", 5)) {
            char* p = buf + 5;
            p[strcspn(p, "\r\n")] = 0; // 개행 제거
            strcpy(user, p);
            // 서버 -> 331
            char resp[128];
            sprintf(resp, "331 Password required for %s", user);
            write(connfd, resp, strlen(resp));
        }
        else if (!strncmp(buf, "PASS ", 5)) {
            char* p = buf + 5;
            p[strcspn(p, "\r\n")] = 0;
            strcpy(pass, p);
            // check_user_pass()
            if (check_user_pass(user, pass)) {
                // OK
                login_ok = 1;
                write(connfd, "230 User logged in.", 20);
                // 로그에 LOG_IN
                char logmsg[64];
                sprintf(logmsg, "LOG_IN");
                write_log(ip_str, cport, user, logmsg, 1, 0, 0);
            }
            else {
                attempts++;
                if (attempts >= 3) {
                    write(connfd, "530 Failed to log-in", 20);
                    // 로그
                    write_log(ip_str, cport, user, "LOG_FAIL(Login)", 1, 0, 0);
                    return;
                }
                else {
                    write(connfd, "430 Invalid user or password", 29);
                }
            }
        }
        else {
            // 기타
            write(connfd, "430 Invalid usage", 17);
        }
    }
    if (!login_ok) return; // 인증 실패

    // 인증 성공 시점: now handle FTP commands in a loop
    time_t start_time = time(NULL);

    while (1) {
        char buf[MAX_BUF] = { 0 };
        int n = read(connfd, buf, MAX_BUF - 1);
        if (n <= 0) {
            // client disconnect
            break;
        }
        buf[n] = '\0';

        // 로깅 (서버가 명령 수신)
        // 예: "[ip:port] user CWD /home"
        // "Fri May 30 16:01:03 2017 [127.0.0.1:32824] seunghyeon CWD /home"
        write_log(ip_str, cport, user, buf, 1, 0, 0);

        // 명령 파싱
        if (!strncmp(buf, "PWD", 3)) {
            // getcwd
            char cwd[512];
            getcwd(cwd, sizeof(cwd));
            // 서버에서 응답: "257 \"<cwd>\" is current directory"
            char msg[600];
            sprintf(msg, "257 \"%s\" is current directory.\r\n", cwd);

            // 로깅 (서버가 결과 전송)
            // "257 PWD command success..."
            write_log(ip_str, cport, user, msg, 1, 0, 0);

            write(connfd, msg, strlen(msg));
        }
        else if (!strncmp(buf, "QUIT", 4)) {
            // 종료
            char msg[] = "221 Goodbye.\r\n";
            write_log(ip_str, cport, user, msg, 1, 0, 0);
            write(connfd, msg, strlen(msg));
            break;
        }
        else if (!strncmp(buf, "CWD ", 4)) {
            // ...
            // 성공 시 "250 CWD command succeeds."
            // 실패 시 "550 <arg>: Can't find such file or directory."
            char* arg = buf + 4;
            arg[strcspn(arg, "\r\n")] = 0;
            int r = chdir(arg);
            if (r == 0) {
                char msg[128];
                sprintf(msg, "250 CWD command succeeds.\r\n");
                write_log(ip_str, cport, user, msg, 1, 0, 0);
                write(connfd, msg, strlen(msg));
            }
            else {
                char msg[256];
                sprintf(msg, "550 %s: Can't find such file or directory.\r\n", arg);
                write_log(ip_str, cport, user, msg, 1, 0, 0);
                write(connfd, msg, strlen(msg));
            }
        }
        else if (!strcmp(buf, "NLST")) {
            // 디렉토리 목록 반환
            DIR* dp = opendir(".");
            struct dirent* de;
            if (!dp) {
                char msg[] = "550 Failed to open directory.\r\n";
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
                continue;
            }
            char msg[MAX_BUF] = { 0 };
            while ((de = readdir(dp)) != NULL) {
                strcat(msg, de->d_name);
                strcat(msg, "\n");
            }
            closedir(dp);
            write(connfd, msg, strlen(msg));
            write_log(ip_str, cport, user, "NLST success", 1, 0, 0);
        }
        else if (!strncmp(buf, "MKD ", 4)) {
            // 디렉토리 생성
            char* dirname = buf + 4;
            dirname[strcspn(dirname, "\r\n")] = 0;
            if (mkdir(dirname, 0755) == 0) {
                char msg[MAX_BUF];
                snprintf(msg, sizeof(msg), "257 Directory %s created.\r\n", dirname);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
            else {
                char msg[MAX_BUF];
                snprintf(msg, sizeof(msg), "550 Failed to create directory %s.\r\n", dirname);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
        }
        else if (!strncmp(buf, "RMD ", 4)) {
            // 디렉토리 삭제
            char* dirname = buf + 4;
            dirname[strcspn(dirname, "\r\n")] = 0;
            if (rmdir(dirname) == 0) {
                char msg[MAX_BUF];
                snprintf(msg, sizeof(msg), "250 Directory %s removed.\r\n", dirname);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
            else {
                char msg[MAX_BUF];
                snprintf(msg, sizeof(msg), "550 Failed to remove directory %s.\r\n", dirname);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
        }
        else if (!strncmp(buf, "RETR ", 5)) {
            // 파일 전송
            char* filename = buf + 5;
            filename[strcspn(filename, "\r\n")] = 0;
            FILE* fp = fopen(filename, "r");
            if (fp) {
                char file_buf[MAX_BUF];
                size_t read_bytes;
                while ((read_bytes = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
                    write(connfd, file_buf, read_bytes);
                }
                fclose(fp);
                char msg[] = "226 File transfer complete.\r\n";
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
            else {
                char msg[256];
                snprintf(msg, sizeof(msg), "550 %s: File not found.\r\n", filename);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
        }
        else if (!strncmp(buf, "STOR ", 5)) {
            // 파일 업로드
            char* filename = buf + 5;
            filename[strcspn(filename, "\r\n")] = 0;
            FILE* fp = fopen(filename, "w");
            if (fp) {
                char file_buf[MAX_BUF];
                int r;
                while ((r = read(connfd, file_buf, sizeof(file_buf))) > 0) {
                    fwrite(file_buf, 1, r, fp);
                }
                fclose(fp);
                char msg[] = "226 File upload complete.\r\n";
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
            else {
                char msg[256];
                snprintf(msg, sizeof(msg), "550 %s: Cannot create file.\r\n", filename);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
        }
        else if (!strncmp(buf, "RNFR ", 5)) {
            // 파일 이름 변경: 첫 단계 RNFR
            strncpy(oldname, buf + 5, MAX_BUF - 1);
            oldname[strcspn(oldname, "\r\n")] = 0;

            if (access(oldname, F_OK) == 0) {
                char msg[] = "350 Ready for RNTO.\r\n";
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
            else {
                char msg[256];
                snprintf(msg, sizeof(msg), "550 %s: File not found.\r\n", oldname);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
        }
        else if (!strncmp(buf, "RNTO ", 5)) {
            // RNTO 처리 (oldname 사용하여 rename 수행)
            char newname[MAX_BUF];
            strncpy(newname, buf + 5, MAX_BUF - 1);
            newname[strcspn(newname, "\r\n")] = 0;

            if (strlen(oldname) == 0) {
                char msg[] = "503 Bad sequence of commands (RNFR required first).\r\n";
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
            else if (rename(oldname, newname) == 0) {
                char msg[256];
                snprintf(msg, sizeof(msg), "250 File renamed successfully to %s.\r\n", newname);
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
                oldname[0] = '\0'; // oldname 초기화
            }
            else {
                char msg[] = "550 Failed to rename file.\r\n";
                write(connfd, msg, strlen(msg));
                write_log(ip_str, cport, user, msg, 1, 0, 0);
            }
        }
        else {
            // 그 외 간단 처리
            char msg[] = "500 Command not recognized.\r\n";
            write_log(ip_str, cport, user, msg, 1, 0, 0);
            write(connfd, msg, strlen(msg));
        }
    }

    // 여기 도달하면 클라이언트 종료
    time_t end_time = time(NULL);
    int service_sec = (int)(end_time - start_time);

    // "Fri May 30 16:01:03 2017 [ip:port] user LOG_OUT [total service time : ??? sec]"
    char comment[128];
    sprintf(comment, "LOG_OUT [total service time : %d sec]", service_sec);
    write_log(ip_str, cport, user, comment, 1, 0, 0);
}

// --------------------------------------------------------
// IP 접근 체크 (access.txt)
int check_access_ip(const char* ip_str)
{
    FILE* fp = fopen(ACCESS_TXT, "r");
    if (!fp) {
        // access.txt가 없으면 일단 전부 거부 (또는 전부 허용)
        return 0;
    }
    char line[IP_BUF];
    while (fgets(line, sizeof(line), fp)) {
        // 줄바꿈 제거
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '\0') continue;

        // 와일드카드(*) 가능. ex) "192.168.*.*"
        // 간단 구현: '.'단위로 토큰 나눠서 4개 대조
        char copy_line[IP_BUF];
        strcpy(copy_line, line);
        char* tok[4];
        tok[0] = strtok(copy_line, ".");
        for (int i = 1; i < 4; i++) tok[i] = strtok(NULL, ".");

        // ip_str도 '.'단위 분할
        char tmp[IP_BUF];
        strcpy(tmp, ip_str);
        char* iptok[4];
        iptok[0] = strtok(tmp, ".");
        for (int i = 1; i < 4; i++) iptok[i] = strtok(NULL, ".");

        int matched = 1;
        for (int i = 0; i < 4; i++) {
            if (!tok[i] || !iptok[i]) {
                matched = 0; break;
            }
            if (strcmp(tok[i], "*") != 0 && strcmp(tok[i], iptok[i]) != 0) {
                matched = 0; break;
            }
        }
        if (matched) {
            fclose(fp);
            return 1; // 허용
        }
    }
    fclose(fp);
    return 0; // 거부
}

// --------------------------------------------------------
// 사용자/비번 검사 (passwd)
int check_user_pass(const char* user, const char* pass)
{
    // 예: passwd 파일에 "seunghyeon:1234" 같은 식
    FILE* fp = fopen(PASSWD_TXT, "r");
    if (!fp) return 0;
    char line[128];
    int ok = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        char* delim = strchr(line, ':');
        if (!delim) continue;
        *delim = 0; // user:pass -> user\0pass
        const char* fuser = line;
        const char* fpass = delim + 1;
        if (!strcmp(user, fuser) && !strcmp(pass, fpass)) {
            ok = 1; break;
        }
    }
    fclose(fp);
    return ok;
}

// --------------------------------------------------------
// 시그널 핸들러들
void sigchld_handler(int signo)
{
    // 자식 종료 시 회수
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

void sigint_handler(int signo)
{
    // 서버 종료 로깅
    write_log(NULL, 0, NULL, "Server is terminated", 1, 0, 0);
    // 종료
    // 부모가 listen 중이라면 적절히 close 후 exit
    // 실제 사용 시 close등 처리
    exit(0);
}

// --------------------------------------------------------
// 로깅 함수
// show_time=1이면 현재 시각 표시, show_service_time 있으면 추가
void write_log(const char* ip, int port, const char* user,
    const char* comment, int show_time, int show_service_time, time_t start_time)
{
    FILE* fp = fopen(LOG_PATH, "a");
    if (!fp) return;

    // 현재 시각
    time_t t = time(NULL);
    struct tm* tm_p = localtime(&t);

    char timestr[128];
    if (show_time) {
        // 예: "Fri May 30 16:01:03 2017"
        strftime(timestr, sizeof(timestr), "%c", tm_p); // %c => OS locale dependent
    }
    else {
        timestr[0] = '\0';
    }

    if (ip && ip[0] != '\0') {
        if (user && user[0] != '\0') {
            // ex) "Fri May 30 16:01:03 2017 [127.0.0.1:32824] k001 some comment..."
            fprintf(fp, "%s [%s:%d] %s %s\n",
                timestr, ip, port, user, comment);
        }
        else {
            // user empty
            fprintf(fp, "%s [%s:%d]  %s\n",
                timestr, ip, port, comment);
        }
    }
    else {
        // ip가 NULL -> 서버시작,서버종료 등
        fprintf(fp, "%s %s\n", timestr, comment);
    }

    fclose(fp);
}

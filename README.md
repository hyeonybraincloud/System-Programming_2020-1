# 시스템 프로그래밍(System Programming - C, Linux)
## Introduction to the Project
### 1. Purpose
소켓 프로그래밍(Socket Programming)을 기반으로, 파일 전송 프로토콜(FTP)을 구현한 Server와 Client를 개발.

### 2. Function and Implementation
- Server가 다중 사용자를 처리할 수 있도록 멀티 프로세스 구조로 구성

- 사용자 및 비밀번호(passwd.txt) 검증을 통한 사용자 인증 기능 제공

- access.txt 파일로 허용된 IP 주소를 관리하여 접속 제어 수행

- Client은 사용자가 입력한 명령어를 FTP 명령어로 변환하여 Server로 전송

- Server은 명령어를 처리한 뒤 결과를 Client에 반환

- 주요 명령어

|Commands|FTP Commands|Description|
|------|---|---|
|ls|NLST|Name List|
|ls -a|NLST -a|Name List all|
|ls -l|NLST -l|Name List Long Format|
|ls -al|NLST -al|Name List All and Long Format|
|dir|LIST|Directory Entry List|
|pwd|PWD|Print Current Working Directory|
|cd|CWD|Change Working Directory|
|cd ..|CDUP|Change Directory to ..|
|mkdir|MKD|Make Directory|
|delete|DELE|Remove File|
|rmdir|RMD|Remove Directory|
|rename|RNFT & RNTO|Rename from and to|
|quit|QUIT|Terminate the command connection|

- Server은 접속 시간, Client IP 및 Port, 사용자의 명령어 및 처리 결과 등 주요 이벤트를 logfile에 기록

### 3. Configuration
![image](https://github.com/user-attachments/assets/6ead9eeb-dc66-4e0f-aaf0-9fcb094f9ca0)

# hyeonybraincloud-System-Programming_2020-1
## Assignment 2: FTP Command Implementation
### 1. Introduction
다음의 명령어를 client와 server 각각을 중심으로 구현하도록 하는 시스템을 설계 및 구현한다.
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

### 2. Flowchart
\- cli.c
 
![image](https://github.com/user-attachments/assets/6fbbb25c-5300-4046-a4be-16ee5c3d2beb)

\- srv.c
![image](https://github.com/user-attachments/assets/37a6516a-7d84-48a5-9810-717e2475170e)


# System Programming
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
\- cli.c: user command을 FTP command로 변환
 
![image](https://github.com/user-attachments/assets/6fbbb25c-5300-4046-a4be-16ee5c3d2beb)

\- srv.c: FTP command을 확인하고 그 명령어를 수행

![image](https://github.com/user-attachments/assets/37a6516a-7d84-48a5-9810-717e2475170e)

### 3. Result
\- PWD

![image](https://github.com/user-attachments/assets/bfdf2903-578c-4ded-8dff-2055d2256751)


\- MKD

![image](https://github.com/user-attachments/assets/1f99e995-7b51-4f5d-95f6-d9ee636ba719)

\- CD

![image](https://github.com/user-attachments/assets/0eba42d9-4468-4acc-93df-b6fef21e135d)

\- CDUP

![image](https://github.com/user-attachments/assets/d053fbb6-b6bb-4f5f-bc57-6f6be1581c48)

\- RNFR & RNTO

![image](https://github.com/user-attachments/assets/c58012b6-8a13-4ae6-a2be-d65fdea5bfff)

\- RMD

![image](https://github.com/user-attachments/assets/e23f9ec6-2315-4d8d-bb6d-38d4644b2453)

\- DELE

![image](https://github.com/user-attachments/assets/e8d504a0-8809-491e-bf3f-f79afdbf3874)

\- QUIT

![image](https://github.com/user-attachments/assets/2d177723-1d06-4d7b-bce4-d971b6f4142e)

\- NLST

![image](https://github.com/user-attachments/assets/df453a68-d573-4d54-baee-b8eba066cdb8)

\- NLST \-a

![image](https://github.com/user-attachments/assets/e0484ab1-59c7-4ea7-864e-300ef3f0b018)

\- NLST \-l

![image](https://github.com/user-attachments/assets/96863d13-f12c-4a8b-9260-565522049734)

\- NLST \-al

![image](https://github.com/user-attachments/assets/27d6d53d-390f-4173-b63a-548baaa77bf8)

\- NLST \[directory] \[option]

![image](https://github.com/user-attachments/assets/5688fd9a-e568-4a57-b369-a985f5ceb3ff)

\- NLST \[file] \[option]

![image](https://github.com/user-attachments/assets/3614b4f1-ff7c-4f1e-a6bf-f2b0d1c36ff0)

## Assignment 3: 

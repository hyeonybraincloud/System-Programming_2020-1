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

![PWD](https://github.com/user-attachments/assets/35c6c0f1-545f-4ba3-8ef2-d239f46965e3)

\- MKD

![MKD](https://github.com/user-attachments/assets/12707231-9f76-4d46-966c-b3971dc45ea6)

\- CD

![CD](https://github.com/user-attachments/assets/d7fe1335-e468-4554-a59a-eed4df5097e6)

\- CDUP

![CDUP](https://github.com/user-attachments/assets/bc9a8716-19ff-4c30-97cb-0361e6171d17)

\- RNFR & RNTO

![RNFR   RNTO](https://github.com/user-attachments/assets/d379b1f2-4f49-4d8a-bf7a-7c11c5222dcc)

\- RMD

![RMD](https://github.com/user-attachments/assets/ad7c5748-a0ab-457e-946b-05970b28c41b)

\- DELE

![DELE](https://github.com/user-attachments/assets/74df07b7-37a1-4b8f-abd1-22f38715aa2c)

\- QUIT

![QUIT](https://github.com/user-attachments/assets/d399a05b-3db2-4f87-893f-28cd434a10a8)

\- NLST

<img width="285" alt="NLST" src="https://github.com/user-attachments/assets/491b916d-8aaa-4ee5-bb63-519387e5f667" />

\- NLST \-a

<img width="292" alt="NLST -a" src="https://github.com/user-attachments/assets/2d8e985d-2f4e-4883-8a59-c3fb04636088" />

\- NLST \-l

<img width="293" alt="NLST -l" src="https://github.com/user-attachments/assets/449a6ee5-0bc4-471b-b957-12d1072884f1" />

\- NLST \-al

<img width="299" alt="NLST -al" src="https://github.com/user-attachments/assets/e9088578-b885-4d28-902c-66bdbad286b2" />

\- NLST \[directory] \[option]

![NLST  directory   option](https://github.com/user-attachments/assets/d270e770-1274-4b34-94e8-a16de6076d52)

\- NLST \[file] \[option]

![NLST  file   option](https://github.com/user-attachments/assets/b1e5e4a6-c5a8-472c-8c12-0b906e35542b)

## Assignment 3: Socket
### 1. Introduction
앞서 했던 방식은 client와 server가 pipe을 이용하여 서로 통신하였다. 그런데 pipe 방식은 local process 간 통신만 가능하며, 네트워크를 통한 원격 통신은 불가능하다. 또한, 기본적으로 pipe 방식은 단방향으로 동작하며, 양방향 통신을 위해서는 추가적인 설정이 필요하다. 그리고 다수의 프로세스를 동시에 처리하는 데 부적합하며, 다중 연결 지원이 어렵다. 이에 대한 대안으로 **socket**이 제안된다. 본 과제에서는 client와 server가 socket을 통해 서로 통신하도록 한다. 이는 client와 server가 네트워크를 통해 **원격 시스템 간 통신을 가능**하게 한다. 또한, **양방향 통신**을 지원하여 **데이터 송수신이 동시에 가능**하다.

### 2. Flowchart
![image](https://github.com/user-attachments/assets/5e7dc2df-7cc5-46f7-a6ae-bf8eca7515d4)

### 3. Result
\- 다중 접속

![image](https://github.com/user-attachments/assets/86ad7a3f-d064-4a22-bce3-7f496b0c32dd)

\- NLST

![image](https://github.com/user-attachments/assets/d02595b1-7c69-4952-9a1a-0de7f9d43549)

\- LIST

<img width="469" alt="스크린샷 2025-01-07 195828" src="https://github.com/user-attachments/assets/deb7e27f-2632-4d91-865f-ac833a9c1b91" />

\- PWD & CND

<img width="323" alt="스크린샷 2025-01-08 001939" src="https://github.com/user-attachments/assets/902c695e-dac2-4c03-8a55-a45c94a59728" />

\- MKD, RNFR & RNTO, RMD

<img width="266" alt="스크린샷 2025-01-08 002123" src="https://github.com/user-attachments/assets/ba8bc06b-dba8-40ef-bd4c-cadb808a74c8" />

\- DELE

<img width="270" alt="스크린샷 2025-01-08 002252" src="https://github.com/user-attachments/assets/4e95a981-7956-494f-8b3e-f8317574c9e6" />

\- QUIT(by command)

<img width="270" alt="스크린샷 2025-01-08 002402" src="https://github.com/user-attachments/assets/0f41433b-acf4-41a1-a5f6-e378c7e614ac" />

\- QUIT(by signal)

<img width="276" alt="스크린샷 2025-01-08 002510" src="https://github.com/user-attachments/assets/ce4e5ea1-5343-4f76-9734-3feb22b0ac6f" />

\- 잘못된 입력에 대한 예외 처리

<img width="313" alt="스크린샷 2025-01-08 002616" src="https://github.com/user-attachments/assets/b92b34cc-a9a9-42cb-829a-4ff7c5bf3fc7" />

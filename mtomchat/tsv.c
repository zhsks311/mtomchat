#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>
#define MAXC 10

int client_array[MAXC];
int clNum=0;
int server_sockfd = 0, client_sockfd = 0;
pthread_mutex_t mutx;

void sig_handler(int signo);
void* clientConnection(void* s);
void sendMsg(char* msg);

int main(void)
{
	signal(SIGINT, (void *)sig_handler);
	int i = 0;
	int server_len=0, client_len=0;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	pthread_t pt;

	for(i=0;i<MAXC;i++)
		client_array[i]=-1;

	if(pthread_mutex_init(&mutx,NULL))
		printf("mutex error\n");

	// 이름없는 소켓 생성
	server_sockfd = socket(AF_INET,SOCK_STREAM, 0);

	// 소켓에 이름 붙이기 
	memset(&server_address,0,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = 9735;
	server_len = sizeof(server_address);

	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	printf("서버가 생성됐습니다.\n");	

	if(listen(server_sockfd,10) == -1)
	{
		printf("listen error\n");
		exit(1);
	}
	printf("연결준비 완료\n");

	// 쓰레드 총 두 개로 구성
	// 메인 함수의 쓰레드는 새로운 연결을 계속 받으며
	// 연결에 성공하면 새로운 쓰레드 생성
	// 새로운 쓰레드는 해당 클라이언트의 입력을 받아와
	// 모든 클라이언트들에게 채팅을 뿌려준다. 
	
	while(1)
	{
		client_len = sizeof(client_sockfd);
		
		client_sockfd = accept(server_sockfd, 
			(struct sockaddr *)&client_address, &client_len);
                                if(client_sockfd == -1)
                                 {
                                        printf(" 연결 수락 실패 \n");
                                        exit(1);
                                 }


		pthread_mutex_lock(&mutx);
		for(i=0;i<MAXC;i++)
		{	
			if(client_array[i] == -1)
			{
				client_array[i] = client_sockfd;
				clNum=i;
				break;
			}
		}
		pthread_mutex_unlock(&mutx);
		pthread_create(&pt, NULL, clientConnection,(void*)&client_sockfd);

	}	
	return 0;
}


void* clientConnection(void *s)
{
	int str_len = 0;
	int tempClient = 0;
	char msg[256];
	char buf[280];
	int i=0;
	int client_sockfd = *(int*) s;
	int clientNumber=clNum+1;
	
	sprintf(buf,"접속자 %d님이 입장하셨습니다.\n",clientNumber);
	sendMsg(buf);
	printf("접속자 %d 생성\n",clientNumber);

	memset(buf,'\0',sizeof(buf));
	memset(msg,'\0',sizeof(msg));
	
	while((str_len = read(client_sockfd, msg,sizeof(msg)) > 0))
	{
		sprintf(buf, "접속자 %d : %s\n", clientNumber, msg);
		sendMsg(buf);
		puts(buf);
		if(strstr(msg,"exit")!=NULL){	
	         	break;
       		}
       	
		
		memset(buf,'\0',sizeof(buf));
		memset(msg,'\0',sizeof(msg));
	}

	pthread_mutex_lock(&mutx);
	for(i =0; i<MAXC; i++)
	{
		if(client_array[i] == client_sockfd)
		{
			tempClient = client_sockfd;
			printf("이거 종료한다. %d\n",i);
			break;
		}

	}
	pthread_mutex_unlock(&mutx);
	close(tempClient);
	client_array[i]=-1;
	printf("접속자 %d 연결종료\n",clientNumber);
	sprintf(buf,"접속자 %d님이 퇴장하셨습니다.\n", clientNumber);
	sendMsg(buf);
	return 0;		
}

void sendMsg(char *msg)
{
	int i=0;
	pthread_mutex_lock(&mutx);
	for(i = 0; i< MAXC;i++){
		if(client_array[i] != -1)
			write(client_array[i], msg, strlen(msg));
	}
	pthread_mutex_unlock(&mutx);

}

void sig_handler(int signo)
{
	int i = 0;
	pthread_mutex_lock(&mutx);
	for(i =0; i < MAXC; i++)
	{
		if(client_array[i] != -1)
		{

			close(client_array[i]);
			client_array[i]=-1;
			printf("%d번째 클라이언트 종료\n",i);
		}	
	}
	pthread_mutex_unlock(&mutx);
	close(server_sockfd);
	exit(0);
}

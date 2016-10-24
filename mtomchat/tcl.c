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


void* say(void* s);
void* hear(void* s);

pthread_t sayPt,hearPt;

int main(void)
{
	int sockfd = 0;
	int len = 0;
	struct sockaddr_in address;
	int result = 0;
	extern pthread_t sayPt,hearPt;		
	// 클라이언트용 소켓 생성

	sockfd = socket(PF_INET,SOCK_STREAM,0);

	
	// 클라이언트와 정해둔 소켓의 이름
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = 9735;
	len = sizeof(address);
	result = connect(sockfd, (struct sockaddr *)&address, len);
        if(result == -1)
        {
                perror("클라이언트 에러 ");
                exit(1);
       }
	system("clear");
	pthread_create(&sayPt,NULL, say, (void*) &sockfd);
	pthread_create(&hearPt,NULL,hear,(void*) &sockfd);
	
	pthread_join(sayPt,NULL);
	pthread_join(hearPt,NULL);

	close(sockfd);
	return 0;
}


void* say(void* s)
{
	char msg[256];
	int str_len=0;
	int sockfd = *(int *)s;
	memset(msg,'\0',sizeof(msg));
	while(1)
	{
		scanf("%s",msg);
		write(sockfd,msg,strlen(msg));	

		
		if(!strncmp(msg,"exit",strlen("exit")))
		{	
			pthread_kill(hearPt,SIGINT);
			break;
		}
		memset(msg,'\0',sizeof(msg));
	}

}

void* hear(void* s)
{

	char msg[256];
	int str_len=0;
	int sockfd = *(int*) s;

	while((str_len = read(sockfd,msg,sizeof(msg))) > 0)
	{
		printf("%s\n",msg);
		if(strstr(msg,"!exit") != NULL)
		{
			pthread_kill(sayPt,SIGINT);
			close(sockfd);
			
			break;
		}
		memset(msg,'\0',sizeof(msg));
	}
	if(str_len ==0)
	{	
		pthread_kill(sayPt,SIGINT);
		close(sockfd);
		pthread_cancel(hearPt);
	}

}

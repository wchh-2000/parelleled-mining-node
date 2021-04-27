//https://www.cnblogs.com/kuangke/p/9397698.html
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h> 
#pragma comment(lib, "ws2_32.lib")
#include <pthread.h>
#pragma comment(lib,"x64/pthreadVC2.lib")
#include "sha256.h"
#include "mining.h"
#define N0 16  //hash开头有N0个2进制0 
void writefile(BYTE *str,int num);
int find=0;
int main(int argc, char *argv[]) {
	WORD socketVersion = MAKEWORD(2,2);
    WSADATA wsaData;
    if(WSAStartup(socketVersion, &wsaData) != 0)
    {
        return 0;
    }
    SOCKET sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(6666);
    sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//本机ip 127.0.0.1  255.255.255.255
    int nAddrLen = sizeof(sin);
    char recvData[blockBytes];
    char sendstr[10];
    unsigned long ul=1;//用于设置非阻塞 允许非阻塞模式则非零
	if(SOCKET_ERROR==ioctlsocket(sclient,FIONBIO,(unsigned long *)&ul))//设置成非阻塞模式
		puts("nonblock failed\n");
	/*-----------------------------------------------------------------------*/
	int i=0,j,len,N=10,prepared=1,verify=0;
	unsigned char hash[32]={0}; //256 bits hash  0用于给第一个区块赋值 
	BLOCK *pre,*h;//前区块指针 区块链头指针 
	BLOCK block[N];
	pthread_t mining_p;
	Arg arg;
	sendto(sclient,"node2",5, 0, (struct sockaddr *)&sin, nAddrLen);
	Sleep(1);
	sendto(sclient,"prepared",8, 0, (struct sockaddr *)&sin, nAddrLen);
    while(i<N)//生成10个区块 
    {
        len = recvfrom(sclient, recvData, blockBytes, 0, (struct sockaddr *)&sin, &nAddrLen);
        if (len > 0)
        {
        	//printf("%s\n",recvData);
            if(prepared && strstr(recvData,"prepared")!=NULL)
            {
            	arg.block=&block[i];
				arg.hash=hash;
				arg.n0=N0;
            	pthread_create(&mining_p,NULL,Mining_thread,(void *)(&arg));//创建挖矿线程 
            	//puts("prepared for next block");
            	prepared=0;
            }
            if(len==blockBytes)//有人挖到，收到区块数据str
            {
            	pthread_cancel(mining_p);
            	for(j=0;j<blockBytes;j++)
            		arg.str[j]=recvData[j];	
				/*
            	puts("receive block:");
            	int j;
            	for(j=0;j<blockBytes;j++)
            		printf("%x ",arg.str[j]);//*/
            	printf("\nreceive block, nonce1:%lld nonce2:%lld\n",str2long(&arg.str[40]),str2long(&arg.str[48]));
            	if(Verify(arg.str,hash,N0))//校验 
            	{
            		verify=1;
            		sendto(sclient,"verified",8, 0, (struct sockaddr *)&sin, nAddrLen);
            		puts("verified");
            	}
            	else
            	{
            		verify=-1;
            		sendto(sclient,"verification failed",19, 0, (struct sockaddr *)&sin, nAddrLen);
					puts("verification failed");
            	}
            }
            if(strstr(recvData,"verified")!=NULL) 
            	verify=1;
            if(strstr(recvData,"verification failed")!=NULL) 
            	verify=-1;
        }
		if(find)
		{
			sendto(sclient,arg.str,blockBytes, 0, (struct sockaddr *)&sin, nAddrLen);
			find=0;
		}
        if(verify==1)//校验成功 
        {
        	printf("\ninfo of %d th block:\n",i+1);
			print_block(arg.str);
			printf("hash:");
			for(j=0;j<4;j++)
				printf("%x ",hash[j]);
			puts("\n");
			writefile(block,++i);
			verify=0;
			prepared=1;
			sendto(sclient,"prepared",8, 0, (struct sockaddr *)&sin, nAddrLen);
			
        }
        if(verify==-1)
        {
        	verify=0;
			prepared=1;
			sendto(sclient,"prepared",8, 0, (struct sockaddr *)&sin, nAddrLen);
			i++;
        }
    }
    closesocket(sclient);
    WSACleanup();
	system("pause"); 
	return 0;
}
void writefile(BYTE *str,int num)//num为块序号 
{
	char s[]="blockchain_data/block";
	char n[4];
	itoa(num,n,10);
	strcat(s,n);
	FILE *fp=fopen(s,"wb");
	if(fp==NULL)
	{
	printf("can't open the file\n");
	exit(0);
	}
	fwrite(str,blockBytes,1,fp);//每个区块信息存储到二进制文件 block->next不存，无意义 	
	//blockBytes=sizeof(BLOCK)-8  block->next(4bytes) 由于struct对齐，占8 
	fclose(fp);
}

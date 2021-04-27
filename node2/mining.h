#ifndef mining_h
#define mining_h

#include "sha256.h"
#include "ecc.h"

#define blockBytes 816 //ÿ��������ռ�ֽ��� 96+transaction*n ����ָ��next���������hash  
#define transBytes 80 //ÿ�ʽ��׼�¼��ռ�ֽ��� ��������ǩ�� 
#define MaxNum 10000 //��nonce1,nonce2������� 
//typedef unsigned char BYTE;             // 8-bit byte

typedef struct {
	BYTE from[33];//264bits ��Դ�߹�Կ 
	BYTE to[33];//264 ȥ���߹�Կ 
	double amount;//64 ת�Ʊ���
	//char time[16];//eg: 2021/03/30/16:05 //��������ע���transBytes
	BYTE signature[64];//512 ecc����ǩ��  
}Transaction; //1104 bits 138 ʵ��ռ144bytes(33+33+6Ϊ8�ı��� 64λ���ݿ��64bits 8 bytes) 
typedef struct BLOCK{
	BYTE prev_hash[32];//256
	long long chain_version;//64 bits
	long long nonce1;//64
	long long nonce2;//64
	double award;//64 �ڿ��� 
	BYTE coinbase[32];//256 �ڿ��߹�Կ
	Transaction trans[5];//n�ʽ��״��Ϊһ������  
	struct BLOCK *next;//(4bytes) ����struct���룬ռ8bytes  
} BLOCK;//824 bytes
typedef struct Arg{
	BLOCK *block;
	BYTE *hash;
	BYTE str[blockBytes];//�洢һ�����鹹���ַ��� 
	int n0;
}Arg;//Mining_thread�̲߳��� 
void *Mining_thread(void *arg);
void Block_init(BLOCK *p,BYTE *hash);
int Verify(BYTE *str,BYTE *hash,int n);
int Mining(BLOCK *p,BYTE *hash,BYTE *str,int n0);//�ҵ�nonce1 nonce2 ʹ��block��hash���㿪ͷ��n0��2����0 
long long str2long(BYTE *s);
void print_block(BYTE *str);

#endif  

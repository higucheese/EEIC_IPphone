#define TIME

#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include <time.h>
#include <sys/time.h>

#define SERVERMODE 2
#define CLIENTMODE 3
#define PORT_OFFSET 1000
#define N 64

const char REC[] = "rec -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null";
const char PLAY[] = "play -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null";

void die(char *str){
	perror(str);
	exit(1);
}

typedef struct{
	int mysocket;
	struct sockaddr_in addr;
} operator/*通話している両者*/;

/*
  クライアントとしての動作をまとめたもの
  connectが終了するまで抜けない
  異常があるときは-1を返す
*/
int initialize_client(operator* o, uint16_t portnum, const char* IP){
	o->mysocket = socket(PF_INET, SOCK_STREAM, 0);
	o->addr.sin_family = AF_INET;
	if(inet_aton(IP, &o->addr.sin_addr) == 0) return -1;
	o->addr.sin_port = htons(portnum);
	return connect(o->mysocket, (struct sockaddr*)&o->addr, sizeof(o->addr));
}

/*
  サーバーとしての動作をまとめたもの
  acceptが終了するまで抜けない
*/
void initialize_server(operator* o, uint16_t portnum){
	int servsocket = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portnum);
	addr.sin_addr.s_addr = INADDR_ANY; //どのIPアドレスでも待ち受けます
	bind(servsocket, (struct sockaddr *)&addr, sizeof(addr));
	listen(servsocket, 10);
	socklen_t len = sizeof(struct sockaddr_in);
	o->mysocket = accept(servsocket, (struct sockaddr *)&o->addr, &len);
	close(servsocket);
}

void *transfer_thread(void *tp){
	operator* transmitter = (operator*) tp;
	FILE *recfp = popen(REC, "r");
	if(recfp == NULL) die("recfp");
	char data[N];
	int n, flag;
	while(1){
		n = (int)fread(data, sizeof(char), N, recfp);
		if(n < N) break;
		flag = (int)send(transmitter->mysocket, data, (size_t)n, 0);
		if(flag == -1) die("send error!");
	}
	pclose(recfp);
	close(transmitter->mysocket);
	pthread_exit(0);
}

void *receive_thread(void *rp){
	operator* receiver = (operator*) rp;
	FILE *playfp = popen(PLAY, "w");
	if(playfp == NULL) die("playfp");
	char data[N];
	int n, flag;
	struct timeval starttime, endtime;
	while(1){
		#ifdef TIME
		gettimeofday(&starttime, NULL);
		#endif
		flag = (int)recv(receiver->mysocket, data, N, 0);
		if(flag == -1) die("send error!");
		n = (int)fwrite(data, sizeof(char), N, playfp);
		if(n < N) break;
		#ifdef TIME
		gettimeofday(&endtime, NULL);
		time_t diffsec = difftime(endtime.tv_sec, starttime.tv_sec);
		suseconds_t diffsub = endtime.tv_sec - starttime.tv_sec;
		double realsec = diffsec + diffsub*1e-6;
		if(realsec >= 0.000001)printf("receive session time:%lf\n", realsec);
		#endif
	}
	pclose(playfp);
	close(receiver->mysocket);
	pthread_exit(0);
}

/*hoge.out PORT IP*/
int main(int argc, char *argv[]){
	if(argc != SERVERMODE && argc != CLIENTMODE) die("hoge.out PORT IP");

	char *PORT = argv[1], *IP = NULL;
	uint16_t port1, port2;
	port1 = (uint16_t)atoi(PORT);
	port2 = (uint16_t)(atoi(PORT) + PORT_OFFSET);
	if(port1 == 0 || port2 == 0) die("portnum");
	operator transmitter, receiver;
	switch(argc){
	case SERVERMODE:
		initialize_server(&transmitter, port1);
		initialize_server(&receiver, port2);
		break;
	case CLIENTMODE:
		IP = argv[2];
		if(initialize_client(&receiver, port1, IP) == -1){
			die("client recv");
		}
		if(initialize_client(&transmitter, port2, IP) == -1){
			die("client trans");
		}
		break;
	default:
		die("mode select");
		break;
	}
	printf("connected\n");
	int one = 1;
	if(setsockopt(transmitter.mysocket, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1
	   || setsockopt(receiver.mysocket, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1)
		die("setsockopt");
	   
	pthread_t trans_t, recv_t;
	pthread_create(&trans_t, NULL, &transfer_thread, &transmitter);
	pthread_create(&recv_t, NULL, &receive_thread, &receiver);
	
	pthread_join(trans_t, NULL);
	pthread_join(recv_t, NULL);
	return 0;
}

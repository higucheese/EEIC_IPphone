#include "IPhone4blib.h"

const char REC[] = "rec -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null";
const char PLAY[] = "play -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null";

void die(char *str){
	perror(str);
	pthread_exit(0);
}

int initialize_client(operator* o, uint16_t portnum, const char* IP){
	/*
  クライアントとしての動作をまとめたもの
  connectが終了するまで抜けない
  異常があるときは-1を返す
	*/
	o->mysocket = socket(PF_INET, SOCK_STREAM, 0);
	o->addr.sin_family = AF_INET;
	if(IP == NULL || inet_aton(IP, &o->addr.sin_addr) == 0) {
		return -1;
	}
	o->addr.sin_port = htons(portnum);
	return connect(o->mysocket, (struct sockaddr*)&o->addr, sizeof(o->addr));
}

void initialize_server(operator* o, uint16_t portnum){
	/*
  サーバーとしての動作をまとめたもの
  acceptが終了するまで抜けない
	*/
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

void *transfer_thread(void *ad){
	Alldata* alldata_p = (Alldata*)ad;
	FILE *recfp = popen(REC, "r");
	if(recfp == NULL) die("recfp");
	char data[N];
	int n, flag;
	while(1){
		n = (int)fread(data, sizeof(char), N, recfp);
		if(n < N || alldata_p->hangflag == TRUE) break;
		flag = (int)send(alldata_p->transmitter_phone.mysocket, data, (size_t)n, 0);
		if(flag == -1) {
			alldata_p->phoneflag = FALSE;
			alldata_p->hangflag = TRUE;
			die("send error!");
		}
	}
	pclose(recfp);
	close(alldata_p->transmitter_phone.mysocket);
	pthread_exit(0);
}

void *receive_thread(void *ad){
	Alldata* alldata_p = (Alldata*)ad;
	FILE *playfp = popen(PLAY, "w");
	if(playfp == NULL) die("playfp");
	char data[N];
	int n, flag;
	while(1){
		flag = (int)recv(alldata_p->receiver_phone.mysocket, data, N, 0);
		if(flag == -1) {
			alldata_p->phoneflag = FALSE;
			alldata_p->hangflag = TRUE;
			die("send error!");
		}
		n = (int)fwrite(data, sizeof(char), N, playfp);
		if(n < N || alldata_p->hangflag == TRUE) break;
	}
	pclose(playfp);
	close(alldata_p->receiver_phone.mysocket);
	pthread_exit(0);
}

void* listen_thread(void* ad){
	Alldata* alldata_p = (Alldata*)ad;
	uint16_t port1, port2;
	port1 = (uint16_t)atoi(alldata_p->PortNum);
	port2 = port1 + PORT_OFFSET;
	initialize_server(&alldata_p->transmitter_phone, port1);
	initialize_server(&alldata_p->receiver_phone, port2);
	//cancelされる前に接続成功
	if(alldata_p->mode == NORMALMODE){
		alldata_p->mode = SERVERMODE;
		alldata_p->phoneflag = TRUE;
	}
	pthread_exit(0);
}

void* connect_thread(void* ad){
	Alldata* alldata_p = (Alldata*)ad;
	uint16_t port1, port2;
	port1 = (uint16_t)atoi(alldata_p->PortNum);
	port2 = port1 + PORT_OFFSET;
	const char* IPaddress_temp = (const char*)alldata_p->IPaddress;
	int count = 0;
	while(1){
		if(initialize_client(&alldata_p->receiver_phone, port1, IPaddress_temp) != -1) {
			break;
		}else{
			if(count == 3) {
				printf("ErrorRecv:Connection[%s]\n", IPaddress_temp);
				alldata_p->hangflag = TRUE;
				pthread_exit(0);
				break;
			}
			count++;
			sleep(1);
		}
	}
	while(1){
		if(initialize_client(&alldata_p->transmitter_phone, port2, IPaddress_temp) != -1) {
			break;
		}else{
			if(count == 3) {
				printf("ErrorTrans:Connection[%s]\n", IPaddress_temp);
				alldata_p->hangflag = TRUE;
				pthread_exit(0);
				break;
			}
			count++;
			sleep(1);
		}
	}
	alldata_p->phoneflag = TRUE;
	pthread_exit(0);
}

void* phone_thread(void* ad){
	Alldata* alldata_p = (Alldata*)ad;
	pthread_t listen_t, connect_t, trans_t, recv_t;
	while(alldata_p->hangflag == FALSE){
		alldata_p->mode = NORMALMODE;
		alldata_p->phoneflag = FALSE;
		alldata_p->callflag = FALSE;
		pthread_create(&listen_t, NULL, &listen_thread, alldata_p);
		gtk_label_set_text((GtkLabel*)alldata_p->Status, "Waiting");
		while(alldata_p->mode == NORMALMODE){
			if(alldata_p->callflag == TRUE){
				alldata_p->mode = CLIENTMODE;
			}
			if(alldata_p->hangflag == TRUE){
				pthread_cancel(listen_t);
				pthread_join(listen_t, NULL);
				pthread_exit(0);
			}
			sleep(1);
		}
		switch(alldata_p->mode){
		case SERVERMODE:
			break;
		case CLIENTMODE:
			pthread_cancel(listen_t);
			pthread_join(listen_t, NULL);
			pthread_create(&connect_t, NULL, &connect_thread, alldata_p);
			gtk_label_set_text((GtkLabel*)alldata_p->Status, "Connecting");
			while(alldata_p->phoneflag == FALSE){
				if(alldata_p->hangflag == TRUE){
					pthread_cancel(connect_t);
					pthread_join(connect_t, NULL);
					pthread_exit(0);
				}
			}
			break;
		default:
			pthread_exit(0);
			break;
		}
		
		int one = 1;
		if(setsockopt(alldata_p->transmitter_phone.mysocket, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1
		   || setsockopt(alldata_p->receiver_phone.mysocket, SOL_TCP, TCP_NODELAY, &one, sizeof(int)) == -1)
			die("setsockopt");
		
		if(pthread_create(&trans_t, NULL, &transfer_thread, alldata_p) != 0 ||
		   pthread_create(&recv_t, NULL, &receive_thread, alldata_p) != 0) break;
		gtk_label_set_text((GtkLabel*)alldata_p->Status, "Connected");
		pthread_join(trans_t, NULL);
		pthread_join(recv_t, NULL);
	}
	gtk_label_set_text((GtkLabel*)alldata_p->Status, "Error");
	pthread_exit(0);
}

#ifndef _IPHONE4BLIB_H_
#define _IPHONE4BLIB_H_

/*chat.c*/
void* chatsend_thread(void*);
void* chatrecv_thread(void*);

/*gui.c*/
#include <string.h>
#include <gtk/gtk.h>
#define WIDTH 480
#define HEIGHT 320
void* gui_thread(void*);

/*phone.c*/
//#define _BSD_SOURCE
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

#define NORMALMODE 1
#define SERVERMODE 2
#define CLIENTMODE 3
#define CHAT_PORT_DEFAULT 57844
#define PORT_OFFSET 1000
#define N 32

typedef struct{
	int mysocket;
	struct sockaddr_in addr;
} operator/*通話している両者*/;

void* phone_thread(void*);
void die(char*);

/*common*/
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    /*phone.c*/
	int mode; //clientかserverか未定義か
	operator transmitter_phone, receiver_phone;
    /*gui.c*/
	GtkWidget* Status;
    /*common*/
	const char* Myname;
	char MyIPaddress[16];
	char MyNetmask[16];
	char Broadcast[32];
	int gui_active_flag;
	int argc;
	char** argv;
	int phoneflag; //今相手と接続中か否か
	int callflag; //callボタンを押したか否か
	int sendflag; //sendボタンを押したか否か
	int hangflag; //hang upボタンを押したか否か
	const gchar* IPaddress; //通話相手のIPアドレス
	const gchar* PortNum; //通話相手のポート番号
	GtkTextBuffer* Buffer;
	gchar Chat_Entry[2048]; //チャットの入力内容
	const gchar* Chat_Entry_temp; //チャットの入力内容
	
} Alldata;

#endif

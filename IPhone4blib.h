#ifndef _IPHONE4BLIB_H_
#define _IPHONE4BLIB_H_

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
#define PORT_OFFSET 1000
#define N 32

typedef struct{
	int mysocket;
	struct sockaddr_in addr;
} operator/*通話している両者*/;

void* phone_thread(void*);

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
	const gchar* Name; //通話相手の名前
	const gchar* Chat_Entry; //チャットの入力内容
	
} Alldata;

#endif

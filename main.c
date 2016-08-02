#include "IPhone4blib.h"

#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

char *itoa( int val, char *a, int radix ){
	char *p = a;
	unsigned int v = val;/* 作業用(変換対象の値) */
	int n = 8;/* 変換文字列の桁数記憶用 */
	while(v >= radix){/* 桁数を求める */
		v /= radix;
		// n++;
	}
	p = a + n; /* 最下位の位置から設定する */
	v = val;
	*p = '\0';/* 文字列終端の設定 */
	do {
		--p;
		*p = v % radix + '0';/* 1桁の数値を文字に変換 */
		if(*p > '9') {/* 変換した文字が10進で表現できない場合 */
			*p = v % radix - 10 + 'A'; /* アルファベットを使う */
		}
		v /= radix;
	} while ( p != a);
	return a;
}

char *itoa2( int val, char *a, int radix ){
	char *p = a;
	unsigned int v = val;/* 作業用(変換対象の値) */
	int n = 1;/* 変換文字列の桁数記憶用 */
	while(v >= radix){/* 桁数を求める */
		v /= radix;
		n++;
	}
	p = a + n; /* 最下位の位置から設定する */
	v = val;
	*p = '\0';/* 文字列終端の設定 */
	do {
		--p;
		*p = v % radix + '0';/* 1桁の数値を文字に変換 */
		if(*p > '9') {/* 変換した文字が10進で表現できない場合 */
			*p = v % radix - 10 + 'A'; /* アルファベットを使う */
		}
		v /= radix;
	} while ( p != a);
	return a;
}

void Alldata_initialize(Alldata* alldata, int argc, char** argv){
	alldata->Myname = NULL;
	alldata->gui_active_flag = TRUE;
	alldata->IPaddress = "127.0.0.1";
	alldata->PortNum = "50000";
	alldata->Chat_Entry_temp = NULL;
	alldata->Status = NULL;
	alldata->Buffer = NULL;
	alldata->argc = argc;
	alldata->argv = argv;
	alldata->phoneflag = FALSE;
	alldata->callflag = FALSE;
	alldata->sendflag = FALSE;
	alldata->hangflag = FALSE;
}

int getIPAddress(const char* target) {
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name,target,IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}

int getNetmask(const char* target){
	struct ifreq ifr;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name,target,IFNAMSIZ-1);
	ioctl(fd, SIOCGIFNETMASK, &ifr);
	close(fd);
	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}

void IPtoBroadcast(const char* myIP, const char* myNetmask, char* Broadcast){
	int i, j;
	char ip[4][4], netmask[4][4], broadcast[4][4];
	char ip_bin[9], netmask_bin[9], broadcast_bin[9];

	for(i = 0;i < 4;i++){
		for(j = 0;j < 4;j++){
			ip[i][j] = '\0';
			netmask[i][j] = '\0';
		    broadcast[i][j] = '\0';
		}
	}

	int counter = 0, period = 0, temp = 0;
	while(counter < 16){
		if(myIP[counter] == '\0') break;
		else if(myIP[counter] == '.') {
			period++;
			temp = 0;
		}
		else{
			ip[period][temp] = myIP[counter];
			temp++;
		}
		counter++;
	}
	counter = 0, period = 0, temp = 0;
	while(counter < 16){
		if(myNetmask[counter] == '\0') break;
		else if(myNetmask[counter] == '.') {
			period++;
			temp = 0;
		}
		else{
			netmask[period][temp] = myNetmask[counter];
			temp++;
		}
		counter++;
	}
	for(i = 0;i < 4;i++){
		itoa(atoi(ip[i]), ip_bin, 2);
		itoa(atoi(netmask[i]), netmask_bin, 2);
		for(j = 0;j < 8;j++){
			if(netmask_bin[j] == '1') broadcast_bin[j] = ip_bin[j];
			else broadcast_bin[j] = '1';
		}
		broadcast_bin[8] = '\0';
		int broadcast_temp = 0, base = 128;
		for(j = 0;j < 8;j++) {
			if(broadcast_bin[j] == '1') broadcast_temp += base;
			base /= 2;
		}
		itoa2(broadcast_temp, broadcast[i], 10);
	}

	counter = 0, period = 0, temp = 0;
	while(period < 4){
		if(broadcast[period][temp] != '\0'){
			Broadcast[counter] = broadcast[period][temp];
			temp++;
		}
		else{
			period++;
			temp = 0;
			if(period < 4) Broadcast[counter] = '.';
		}
		counter++;
	}
}

int main(int argc, char* argv[]){
	Alldata alldata; //全体のスレッドで共有する
	Alldata_initialize(&alldata, argc, argv);
	
	struct in_addr inaddr_ip, inaddr_netmask;

	inaddr_netmask.s_addr = getNetmask("wlp1s0");
	strncpy(alldata.MyNetmask, inet_ntoa(inaddr_netmask), 16);

	inaddr_ip.s_addr = getIPAddress("wlp1s0");
	strncpy(alldata.MyIPaddress, inet_ntoa(inaddr_ip), 16);

	IPtoBroadcast(alldata.MyIPaddress, alldata.MyNetmask, alldata.Broadcast);
	//printf("Broadcast:%s\n", alldata.Broadcast);

	pthread_t gui_t, phone_t, chatsend_t, chatrecv_t;
	if(pthread_create(&gui_t, NULL, &gui_thread, &alldata) != 0){
		perror("guiの起動に失敗");
		exit(1);
	}
	if(pthread_create(&chatsend_t, NULL, &chatsend_thread, &alldata) != 0 ||
		pthread_create(&chatrecv_t, NULL, &chatrecv_thread, &alldata) != 0){
		perror("chatの起動に失敗");
		exit(1);
	}
	while(alldata.gui_active_flag == TRUE){
		if(pthread_create(&phone_t, NULL, &phone_thread, &alldata) != 0){
			perror("phoneの起動に失敗");
			exit(1);
		}
		pthread_join(phone_t, NULL);
		alldata.hangflag = FALSE;
	}
	pthread_join(chatsend_t, NULL);
	pthread_cancel(chatrecv_t);
	pthread_join(chatrecv_t, NULL);
	pthread_join(gui_t, NULL);

	return 0;
}

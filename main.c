#include "IPhone4blib.h"

#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

void Alldata_initialize(Alldata* alldata, int argc, char** argv){
	alldata->Myname = NULL;
	alldata->MyIPaddress = NULL;
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

void IPtoBroadcast(const char* myIP, char* Broadcast){
	int counter = 0, period = 0;
	while(counter < 16){
		Broadcast[counter] = myIP[counter];
		if(myIP[counter] == '.') period++;
		if(period == 3){
			Broadcast[counter + 1] = '2';
			Broadcast[counter + 2] = '5';
			Broadcast[counter + 3] = '5';
			Broadcast[counter + 4] = '\0';
			break;
		}
		counter++;
	}
}

int main(int argc, char* argv[]){
	Alldata alldata; //全体のスレッドで共有する
	Alldata_initialize(&alldata, argc, argv);
	
	struct in_addr inaddr;
	int address = 0;
	address = getIPAddress("wlan0");
	inaddr.s_addr = address;
	alldata.MyIPaddress = inet_ntoa(inaddr);
	IPtoBroadcast(alldata.MyIPaddress, alldata.Broadcast);

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

#include "IPhone4blib.h"

void *chatsend_thread(void *ad){
	Alldata* alldata_p = (Alldata*)ad;

	int mysocket = socket(PF_INET, SOCK_DGRAM, 0);
	int broadcastEnable = 1;
	setsockopt(mysocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	struct sockaddr_in addr;

	addr.sin_family = AF_INET; 
	if(inet_aton(alldata_p->MyIPaddress, &addr.sin_addr) == 0) {
		die("IPaddress error!");
	}
	if(alldata_p->argc > 1){
		addr.sin_port = htons((uint16_t)atoi(alldata_p->argv[1]));
	}else{
		addr.sin_port = htons(CHAT_PORT_DEFAULT);
	}
	addr.sin_addr.s_addr = inet_addr(alldata_p->Broadcast);

	char data[2048];
	while(alldata_p->gui_active_flag == TRUE){
		while(alldata_p->sendflag == FALSE) sleep(1);
		if(alldata_p->Chat_Entry != NULL){
			sprintf(data, "[%s](%s):%s\n", alldata_p->Myname, alldata_p->MyIPaddress, alldata_p->Chat_Entry);
			if(sendto(mysocket, data, (int)strlen(data), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0){
				die("sendto");
			}
		}
		alldata_p->sendflag = FALSE;
	}
	close(mysocket);
	pthread_exit(0);
}

void *chatrecv_thread(void *ad){
	Alldata* alldata_p = (Alldata*)ad;
	
	int mysocket = socket(PF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;

	socklen_t sin_size;
    struct sockaddr_in from_addr;
	
	addr.sin_family = AF_INET; 
	if(inet_aton(alldata_p->MyIPaddress, &addr.sin_addr) == 0) {
		die("IPaddress error!");
	}
	if(alldata_p->argc > 1){
		addr.sin_port = htons((uint16_t)atoi(alldata_p->argv[1]));
	}else{
		addr.sin_port = htons(CHAT_PORT_DEFAULT);
	}
	addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(mysocket, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		die("bind");
	}

	char data[2048];
	while(alldata_p->gui_active_flag == TRUE){
		memset(data, '\0', sizeof(data));
		if(recvfrom(mysocket, data, sizeof(data), 0, (struct sockaddr *)&from_addr, &sin_size) < 0){
			die("recvfrom");
		}
		if(data[0] != '\0'){
			gtk_text_buffer_insert_at_cursor((GtkTextBuffer*)alldata_p->Buffer, data, (gint)strlen(data));
		}
	}
	close(mysocket);
	pthread_exit(0);
}

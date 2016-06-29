#include "IPhone4blib.h"

void Alldata_initialize(Alldata*, int, char**);

int main(int argc, char* argv[]){
	Alldata alldata; //全体のスレッドで共有する
	Alldata_initialize(&alldata, argc, argv);
	pthread_t gui_t, phone_t;
	if(pthread_create(&gui_t, NULL, &gui_thread, &alldata) != 0){
		perror("guiの起動に失敗");
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
	pthread_join(gui_t, NULL);
	return 0;
}

void Alldata_initialize(Alldata* alldata, int argc, char** argv){
	alldata->gui_active_flag = TRUE;
	alldata->Name = NULL;
	alldata->IPaddress = "127.0.0.1";
	alldata->PortNum = "50000";
	alldata->Chat_Entry = NULL;
	alldata->Status = NULL;
	alldata->Buffer = NULL;
	alldata->argc = argc;
	alldata->argv = argv;
	alldata->phoneflag = FALSE;
	alldata->callflag = FALSE;
	alldata->sendflag = FALSE;
	alldata->hangflag = FALSE;
}

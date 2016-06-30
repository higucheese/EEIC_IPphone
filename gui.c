#include "IPhone4blib.h"

void ProgramFinish (GtkWidget* widget, gpointer data){
	((Alldata*)data)->gui_active_flag = FALSE;
	((Alldata*)data)->hangflag = TRUE;
	gtk_main_quit();	
}

void NameChange (GtkWidget* widget, gpointer data){
	((Alldata*)data)->Myname = gtk_combo_box_text_get_active_text((GtkComboBoxText*)widget);
}
void IPChange (GtkWidget* widget, gpointer data){
	((Alldata*)data)->IPaddress = gtk_entry_get_text((GtkEntry*)widget);
}
void PortChange (GtkWidget* widget, gpointer data){
	((Alldata*)data)->PortNum = gtk_entry_get_text((GtkEntry*)widget);
	((Alldata*)data)->hangflag = TRUE;
}
void ChatChange (GtkWidget* widget, gpointer data){
	((Alldata*)data)->Chat_Entry_temp = gtk_entry_get_text((GtkEntry*)widget);
}
void CallClick (GtkWidget* widget, gpointer data){
	((Alldata*)data)->callflag = TRUE;
	if(((Alldata*)data)->argc > 1){
		FILE *nc = popen("nc higuberry.mydns.jp 50000", "w");
		fprintf(nc, "%s\n", ((Alldata*)data)->Myname);
		pclose(nc);
	}
}
void HangClick (GtkWidget* widget, gpointer data){
	((Alldata*)data)->hangflag = TRUE;
	((Alldata*)data)->phoneflag = FALSE;
}
void SendClick (GtkWidget* widget, gpointer data){
	((Alldata*)data)->sendflag = TRUE;
	strncpy(((Alldata*)data)->Chat_Entry, ((Alldata*)data)->Chat_Entry_temp, 2047);
}
void SendClick2 (GtkWidget* widget, gpointer data){
	gtk_entry_set_text((GtkEntry*)data, "");
}

void* gui_thread(void* ad){
	Alldata* alldata_p = (Alldata*)ad;
	/*ここからgtkのプログラムを始める*/
	gtk_init(&alldata_p->argc, &alldata_p->argv);
	
	/*windowの設定*/
	GtkWidget *window;
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"IPhone4B");
	gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
	g_signal_connect(window,"destroy", G_CALLBACK(ProgramFinish), alldata_p);

	/*段同士をつなぐ*/
	GtkWidget *vbox;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	/*1段目*/
	GtkWidget *hbox_top, *namelabel, *combo_box, *IPlabel, *IPentry, *Portlabel, *Portentry, *call_button, *hang_button;
	hbox_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); //ボックスサイズを同じにするか, ウィジェット同士の距離
	gtk_box_pack_start(GTK_BOX(vbox), hbox_top, FALSE, FALSE, 0);
	//コンボボックス
	namelabel = gtk_label_new("Name:");
	gtk_box_pack_start(GTK_BOX(hbox_top), namelabel, TRUE, TRUE, 0);
	combo_box = gtk_combo_box_text_new ();
	gtk_combo_box_text_append_text ((GtkComboBoxText*)combo_box, "江藤 亮");
	gtk_combo_box_text_append_text ((GtkComboBoxText*)combo_box, "樋口 兼一");
	gtk_combo_box_text_append_text ((GtkComboBoxText*)combo_box, "柏嶋 始");
	gtk_combo_box_text_append_text ((GtkComboBoxText*)combo_box, "西澤 広貴");
	gtk_combo_box_set_id_column((GtkComboBox*)combo_box, 1);
	gtk_box_pack_start(GTK_BOX(hbox_top), combo_box, TRUE, TRUE, 0);
	//IP入力フォーム
	IPlabel = gtk_label_new("IPv4:");
	gtk_box_pack_start(GTK_BOX(hbox_top), IPlabel, TRUE, TRUE, 0);
	IPentry = gtk_entry_new();
	gtk_entry_set_text((GtkEntry*)IPentry, "127.0.0.1"); //default port num
	gtk_box_pack_start(GTK_BOX(hbox_top), IPentry, TRUE, TRUE, 0);
	//Port入力フォーム
    Portlabel = gtk_label_new("Port:");
	gtk_box_pack_start(GTK_BOX(hbox_top), Portlabel, TRUE, TRUE, 0);
	Portentry = gtk_entry_new();
	gtk_entry_set_text((GtkEntry*)Portentry, "50000"); //default port num
	gtk_box_pack_start(GTK_BOX(hbox_top), Portentry, TRUE, TRUE, 0);
	//ボタン
	call_button = gtk_button_new_with_label("Call");
	hang_button = gtk_button_new_with_label("Hang Up");
	gtk_box_pack_start(GTK_BOX(hbox_top), call_button, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_top), hang_button, TRUE, TRUE, 0);
    
	/*2段目*/
	GtkWidget *frame, *chat_view, *scrolled_window;
	frame = gtk_frame_new("ChatBox");
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    chat_view = gtk_text_view_new();
	alldata_p->Buffer = gtk_text_view_get_buffer((GtkTextView*) chat_view);
	gtk_text_view_set_editable ((GtkTextView*)chat_view, FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), chat_view);
	gtk_container_add(GTK_CONTAINER(frame), scrolled_window);
    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	
	/*3段目*/
	GtkWidget *hbox_bottom, *chat_label, *chat_entry, *send_button;
	hbox_bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_bottom, FALSE, FALSE, 0);
	//entry
	chat_label = gtk_label_new("Entry:");
	gtk_box_pack_start(GTK_BOX(hbox_bottom), chat_label, FALSE, FALSE, 0);
	chat_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox_bottom), chat_entry, TRUE, TRUE, 0);
	//send
	send_button = gtk_button_new_with_label("Send");
	gtk_box_pack_start(GTK_BOX(hbox_bottom), send_button, FALSE, FALSE, 0);

	/*4段目*/
	GtkWidget *hseparator, *hbox_status, *text_label;
	hseparator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(vbox), hseparator, FALSE, FALSE, 0);
	hbox_status = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_status, FALSE, FALSE, 0);
	text_label = gtk_label_new("Telecommunication Status : ");
	alldata_p->Status = gtk_label_new("Not Connected");
	gtk_box_pack_start(GTK_BOX(hbox_status), text_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_status), alldata_p->Status, FALSE, FALSE, 0);

	/*動作*/
	//逐次動作
	g_signal_connect(combo_box, "changed", G_CALLBACK(NameChange), alldata_p);
	g_signal_connect(IPentry, "changed", G_CALLBACK(IPChange), alldata_p);
	g_signal_connect(Portentry, "changed", G_CALLBACK(PortChange), alldata_p);
	g_signal_connect(chat_entry, "changed", G_CALLBACK(ChatChange), alldata_p);	
	//クリック時動作
	g_signal_connect(call_button, "clicked", G_CALLBACK(CallClick), alldata_p);
	g_signal_connect(hang_button, "clicked", G_CALLBACK(HangClick), alldata_p);
	g_signal_connect(send_button, "clicked", G_CALLBACK(SendClick), alldata_p);
	g_signal_connect(send_button, "clicked", G_CALLBACK(SendClick2), chat_entry);
	
	gtk_widget_show_all(window);
	gtk_main();
	pthread_exit(0);
}

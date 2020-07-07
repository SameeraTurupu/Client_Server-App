#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist, *alist, *printid;

struct bufserv{

	int userId;
	int forumId;
	int msgId;
	int commentId;
	int choice;
	char *forumname;
	char msg[128];
}buf1;

struct Date
{
	int d, m, y;
};

const int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
int pre_sys_date;
int pre_sys_month;
int pre_sys_year;
int sys_date;
int sys_month;
int sys_year;
char* reply_buff = (char*)malloc(sizeof(char)* 1024);
char* client_list;
int choosen;
char* client_name = (char*)malloc(sizeof(char)* 32);
int day;
int month;
int year;
int flag = 0;
int count = 0;
int choice = 0;
char* cat_name = (char*)malloc(sizeof(char)* 32);
char* role = (char*)malloc(sizeof(char)* 32);
char* phone = (char*)malloc(sizeof(char)* 16);
int insert_pos;
char* selected_name = (char*)malloc(sizeof(char)* 32);
char** given_list;
int select1;
char* reg = (char*)malloc(sizeof(char)* 32); //register
char* app = (char*)malloc(sizeof(char)* 32); //appointment

int block;
char* data = NULL;

FILE* fp;
char *Data[100];
int values[100];
DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);

void socket_server() {

	//The port you want the server to listen on
	int host_port = 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0 || (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)) {
		fprintf(stderr, "No sock dll %d\n", WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if (hsock == -1){
		printf("Error initializing socket %d\n", WSAGetLastError());
		goto FINISH;
	}

	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1) ||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1)){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(host_port);

	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	/* if you get error in bind
	make sure nothing else is listening on that port */
	if (bind(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1){
		fprintf(stderr, "Error binding to socket %d\n", WSAGetLastError());
		goto FINISH;
	}
	if (listen(hsock, 10) == -1){
		fprintf(stderr, "Error listening %d\n", WSAGetLastError());
		goto FINISH;
	}

	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);

	while (true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));

		if ((*csock = accept(hsock, (SOCKADDR*)&sadr, &addr_size)) != INVALID_SOCKET){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0, 0, &SocketHandler, (void*)csock, 0, 0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n", WSAGetLastError());
		}
	}

FINISH:
	;
}

void put_data(){
	fseek(fp, block, SEEK_SET);
	int i = 0;
	fread(&i, 4, 1, fp);
	if (i == 0){
		fseek(fp, block + 4, SEEK_SET);
		fwrite(cat_name, 32, 1, fp);
		fwrite(role, 32, 1, fp);
		fwrite(phone, 16, 1, fp);
		int j = -1;
		fwrite(&j, 4, 1, fp);
		fwrite(&j, 4, 1, fp);
		int temp = ftell(fp);
		fseek(fp, block, SEEK_SET);
		fwrite(&temp, 4, 1, fp);
	}
	if (i != 0){
		int t = block + 4;
		do{
			fseek(fp, t + 32 + 32 + 16, SEEK_SET);
			fread(&t, 4, 1, fp);
		} while (t != -1);
		fseek(fp, -4, SEEK_CUR);
		fwrite(&i, 4, 1, fp);
		fseek(fp, i, SEEK_SET);
		fwrite(cat_name, 32, 1, fp);
		fwrite(role, 32, 1, fp);
		fwrite(phone, 16, 1, fp);
		int j = -1;
		fwrite(&j, 4, 1, fp);
		fwrite(&j, 4, 1, fp);
		int temp = ftell(fp);
		fseek(fp, block, SEEK_SET);
		fwrite(&temp, 4, 1, fp);
	}
}


char* get_data(){
	data = (char*)malloc(sizeof(char)*(32 + 32 + 16 + 5) * 5);//5 for write 0.\n 1.\n etc
	int size = 5;
	fseek(fp, block, SEEK_SET);
	int i = 0;
	fread(&i, 4, 1, fp);
	int t = block + 4;
	int iter = 0;
	data[0] = '0';
	data[1] = '.';
	data[2] = ' ';
	data[3] = '\0';
	int temp = 1;
	do{
		if (iter == size){
			size *= 2;
			data = (char*)realloc(data, sizeof(char)*(32 + 32 + 16 + 5)*size);
		}
		fseek(fp, t, SEEK_SET);
		fread(cat_name, 32, 1, fp);
		fread(role, 32, 1, fp);
		fread(phone, 16, 1, fp);
		strcat(data, cat_name);
		data = strcat(data, strdup(" "));
		strcat(data, role);
		data = strcat(data, strdup(" "));
		strcat(data, phone);
		data = strcat(data, strdup("\n"));
		int var = temp + 48;
		strcat(data, strdup((char*)&var));
		data = strcat(data, strdup(". "));
		fread(&t, 4, 1, fp);
		iter++;
		temp++;
	} while (t != -1);
	data[strlen(data) - 3] = '\0';
	return data;
}

int valid_date(int dd, int mm, int yy) {
	//int dd = day;
	//int mm = month;
	//int yy = year;
	if (mm < 1 || mm > 12) {
		return 0;
	}
	if (dd < 1) {
		return 0;
	}

	int days = 31;
	if (mm == 2) {
		days = 28;
		if (yy % 400 == 0 || (yy % 4 == 0 && yy % 100 != 0)) {
			days = 29;
		}
	}
	else if (mm == 4 || mm == 6 || mm == 9 || mm == 11) {
		days = 30;
	}

	if (dd > days) {
		return 0;
	}
	return 1;
}

int str_to_num(char* str){
	int i = 0;
	int sum = 0;
	while (str[i]){
		sum = sum * 10 + (str[i] - '0');
		i++;
	}
	return sum;
}

int update_client_data(){
	//check whether the date & month & year in current clients
	int d, m, y;
	fseek(fp, insert_pos, SEEK_SET);
	int t;
	fread(&t, 4, 1, fp);
	int iter = 0;
	int temp = 1;
	while (t != -1){
		fseek(fp, t + 32, SEEK_SET);
		fread(&d, 4, 1, fp);
		fread(&m, 4, 1, fp);
		fread(&y, 4, 1, fp);
		if ((d == day) && (m == month) && (y == year)){
			return 1;
		}
		fread(&t, 4, 1, fp);
	}


	fseek(fp, block, SEEK_SET);
	int off;
	fread(&off, 4, 1, fp);

	fseek(fp, insert_pos, SEEK_SET);
	fread(&temp, 4, 1, fp);
	while (temp != -1){
		fseek(fp, temp + 32 + 4 + 4 + 4, SEEK_SET);
		fread(&temp, 4, 1, fp);
	}
	fseek(fp, -4, SEEK_CUR);
	fwrite(&off, 4, 1, fp);
	fseek(fp, off, SEEK_SET);
	fwrite(client_name, 32, 1, fp);
	fwrite(&day, 4, 1, fp);
	fwrite(&month, 4, 1, fp);
	fwrite(&year, 4, 1, fp);
	int neg1 = -1;
	fwrite(&neg1, 4, 1, fp);
	temp = ftell(fp);
	fseek(fp, block, SEEK_SET);
	fwrite(&temp, 4, 1, fp);
	return 0;
}

char* int_to_str(int num){
	char* rv = (char*)malloc(sizeof(char)* 5);
	int i = 0;
	while (num > 0){
		rv[i] = (num % 10) + '0';
		i++;
		num /= 10;
	}
	rv[i] = '\0';
	return strrev(rv);
}

char* view_clients(){
	client_list = (char*)malloc(sizeof(char)*(32 + 4 + 4 + 4 + 5) * 5);//5 for write 0.\n 1.\n etc
	int size = 5;
	/*
	fseek(fp, block, SEEK_SET);
	int i = 0;
	fread(&i, 4, 1, fp);
	int t = block + 4;
	*/
	fseek(fp, insert_pos, SEEK_SET);
	int t;
	fread(&t, 4, 1, fp);
	int iter = 0;
	client_list[0] = '0';
	client_list[1] = '.';
	client_list[2] = ' ';
	client_list[3] = '\0';
	int temp = 1;
	while (t != -1){
		if (iter == size){
			size *= 2;
			client_list = (char*)realloc(client_list, sizeof(char)*(32 + 32 + 16 + 5)*size);
		}
		fseek(fp, t, SEEK_SET);
		fread(cat_name, 32, 1, fp);
		fread(&day, 4, 1, fp);
		fread(&month, 4, 1, fp);
		fread(&year, 4, 1, fp);
		strcat(client_list, cat_name);
		client_list = strcat(client_list, strdup(" "));
		strcat(client_list, int_to_str(day));
		client_list = strcat(client_list, strdup(" "));
		strcat(client_list, int_to_str(month));
		client_list = strcat(client_list, strdup(" "));
		strcat(client_list, int_to_str(year));
		client_list = strcat(client_list, strdup("\n"));
		int var = temp + 48;
		strcat(client_list, strdup((char*)&var));
		client_list = strcat(client_list, strdup(". "));
		fread(&t, 4, 1, fp);
		iter++;
		temp++;
	}
	if (strlen(client_list) == 3){
		client_list = NULL;
		return NULL;
	}
	else{
		client_list[strlen(client_list) - 3] = '\0';
	}
	return client_list;
}

void get_sys_date(){
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	sys_month = t->tm_mon + 1;
	sys_date = t->tm_mday;
	sys_year = t->tm_year + 1900;
}

void get_pre_sys_date(){
	time_t now = time(NULL);
	now = now - (24 * 60 * 60);
	struct tm *t = localtime(&now);
	pre_sys_month = t->tm_mon + 1;
	pre_sys_date = t->tm_mday;
	pre_sys_year = t->tm_year + 1900;
}


int countLeapYears(Date d)
{
	int years = d.y;

	// Check if the current year needs to be considered
	// for the count of leap years or not
	if (d.m <= 2)
		years--;

	// An year is a leap year if it is a multiple of 4,
	// multiple of 400 and not a multiple of 100.
	return years / 4 - years / 100 + years / 400;
}


int getDifference(Date dt1, Date dt2)
{
	// COUNT TOTAL NUMBER OF DAYS BEFORE FIRST DATE 'dt1'

	// initialize count using years and day
	long int n1 = dt1.y * 365 + dt1.d;

	// Add days for months in given date
	for (int i = 0; i < dt1.m - 1; i++)
		n1 += monthDays[i];

	// Since every leap year is of 366 days,
	// Add a day for every leap year
	n1 += countLeapYears(dt1);

	// SIMILARLY, COUNT TOTAL NUMBER OF DAYS BEFORE 'dt2'

	long int n2 = dt2.y * 365 + dt2.d;
	for (int i = 0; i < dt2.m - 1; i++)
		n2 += monthDays[i];
	n2 += countLeapYears(dt2);

	// return difference between two counts
	return (n2 - n1);
}

int check_date_in_6_months(){
	Date dt1 = { sys_date, sys_month, sys_year };
	Date dt2 = { day, month, year };
	int temp = getDifference(dt1, dt2);
	printf("temp = %d\n", temp);
	if (temp <= 183 && temp >= 0){
		return 1;
	}
	else if (temp < 0){
		return 2;
	}
	return 0;
}


void process_input(char *recvbuf, int recv_buf_cnt, int* csock)
{
	if (count == 0){
		if (choice == 0){
			given_list = (char**)malloc(sizeof(char*)* 10);
			for (int i = 0; i < 10; i++){
				given_list[i] = (char*)malloc(sizeof(char)* 15);
			}
			strcpy(given_list[0], strdup("doctor"));
			strcpy(given_list[1], strdup("technician"));
			strcpy(given_list[2], strdup("plumber"));
			strcpy(given_list[3], strdup("electrician"));
			strcpy(given_list[4], strdup("teacher"));
			strcpy(given_list[5], strdup("driver"));
			strcpy(given_list[6], strdup("chef"));
			strcpy(given_list[7], strdup("security"));
			strcpy(given_list[8], strdup("servant"));
			strcpy(given_list[9], strdup("manager"));

			fp = fopen("calendar.bin", "rb+");
			get_sys_date();
			int d, m, y;
			fread(&d, 4, 1, fp);
			fread(&m, 4, 1, fp);
			fread(&y, 4, 1, fp);
			fseek(fp, 0, SEEK_SET);

			if (!(d == 0 && m == 0 && y == 0)){
				//delete the previous day clients
				for (int i = 0; i < 10; i++){
					fseek(fp, 4 + 4 + 4 + (10485760 * i), SEEK_SET);
					int check;
					fread(&check, 4, 1, fp);
					if (check != 0){
						int temp;
						do{
							fseek(fp, 32 + 32 + 16 , SEEK_CUR);
							int t = ftell(fp);
							fseek(fp, 4, SEEK_CUR);
							int temp1;
							fread(&temp1, 4, 1, fp);
							fseek(fp, -4, SEEK_CUR);
							int pre = ftell(fp);
							get_pre_sys_date();
							while (temp1 != -1){
								fseek(fp, temp1 + 32, SEEK_SET);//skip name of client
								int d1, m1, y1;
								fread(&d1, 4, 1, fp);
								fread(&m1, 4, 1, fp);
								fread(&y1, 4, 1, fp);
								if ((d1 == pre_sys_date) && (m1 == pre_sys_month) && (y1 == pre_sys_year)){
									//delete client
									fread(&temp1, 4, 1, fp);
									fseek(fp, pre, SEEK_SET);
									fwrite(&temp1, 4, 1, fp);
									break;
								}
								pre = ftell(fp);
								fread(&temp1, 4, 1, fp);
							}
							
							fseek(fp, t, SEEK_SET);
							fread(&temp, 4, 1, fp);
							fseek(fp, temp, SEEK_SET);
						} while (temp != -1);
					}
				}
			}
			else{
				fwrite(&sys_date, 4, 1, fp);
				fwrite(&sys_month, 4, 1, fp);
				fwrite(&sys_year, 4, 1, fp);
			}

			choice = 1;
		}
		if (choice == 1){
			select1 = recvbuf[0] - '0';
			strcpy(reg, strdup("Register as "));
			reg = strcat(reg, strdup(given_list[select1]));

			strcpy(app, strdup("1. Appointment for "));
			app = strcat(app, strdup(given_list[select1]));

			block = 10485760 * (recvbuf[0] - '0') + 4 + 4 + 4;
			fseek(fp, block, SEEK_SET);
			int i;
			fread(&i, 4, 1, fp);
			if (i == 0){
				strcpy(reply_buff, strdup("No data found\n========================Menu========================\n"));
				reply_buff = strcat(reply_buff, strdup("\n1. "));
				reply_buff = strcat(reply_buff, reg);
				reply_buff = strcat(reply_buff, strdup("\n2. Back\n3. Exit\nEnter : \n"));
				replyto_client(reply_buff, csock);
			}
			else{
				strcpy(reply_buff, strdup("\n========================Menu========================\n"));
				reply_buff = strcat(reply_buff, app);
				reply_buff = strcat(reply_buff, strdup("\n2. "));
				reply_buff = strcat(reply_buff, reg);
				reply_buff = strcat(reply_buff, strdup("\n3. Back\n4. Exit\nEnter : \n"));
				replyto_client(reply_buff, csock);
				flag = 1;
			}
			count = 1;
			choice = 0;
		}
	}
	else if (count == 1){
		if (choice == 0){
			if (flag == 1){
				if (recvbuf[0] == '1'){
					replyto_client(strdup("\n========================Menu========================\n1. Register as client\n2. View clients\n3. Back\n4. Exit\n"), csock);
					count = 2;
					choice = 0;
				}
				else{
					recvbuf[0] -= 1;
					flag = 0;
				}
			}
			if (flag == 0){
				if (recvbuf[0] == '1'){
					replyto_client(strdup("Enter name\n"), csock);
					choice = 1;
				}
				else if (recvbuf[0] == '2'){

					replyto_client(strdup("0 . doctors\n1 . technicians\n2 . plumbers\n3 . electricians\n4 . teachers\n5 . drivers\n6 . chefs\n7 . security\n8 . servants\n9 . managers\n\nEnter : \n"), csock);
					count = 0;
					choice = 1;
				}
				else{
					replyto_client(strdup("EXIT"), csock);
					exit(0);
				}
			}
			flag = 0;
		}
		else if (choice == 1){
			strcpy(cat_name, recvbuf);
			replyto_client(strdup("Enter role\n"), csock);
			choice = 2;
		}
		else if (choice == 2){
			strcpy(role, recvbuf);
			replyto_client(strdup("Enter phone number\n"), csock);
			choice = 3;
		}
		else if (choice == 3){
			strcpy(phone, recvbuf);
			put_data();
			strcpy(reply_buff, strdup("Successfully submited\n========================Menu========================\n"));
			//reply_buff = strcat(reply_buff, app);
			reply_buff = strcat(reply_buff, strdup("\n1. Back\n2. Exit\nEnter : \n"));
			replyto_client(reply_buff, csock);
			//replyto_client(strdup("Successfully submited\n\n========================Menu========================\n1. Register\n2. Back\n3. Exit\nEnter : \n"), csock);
			choice = 9;
		}
		else if (choice == 4){
			choosen = str_to_num(recvbuf);
			int index = choosen;
			fseek(fp, block, SEEK_SET);
			int temp;
			if (index >= 0){
				fseek(fp, 4, SEEK_CUR);
			}
			while (index > 0){
				fseek(fp, 32 + 32 + 16, SEEK_CUR);
				fread(&temp, 4, 1, fp);
				fseek(fp, temp, SEEK_SET);
				index--;
			}
			fseek(fp, 32 + 32 + 16 + 4, SEEK_CUR);
			insert_pos = ftell(fp);
			replyto_client(strdup("Enter name : "), csock);
			choice = 5;
		}
		else if (choice == 5){
			strcpy(client_name, recvbuf);
			replyto_client(strdup("Enter day : "), csock);
			choice = 6;
		}
		else if (choice == 6){
			day = str_to_num(recvbuf);
			replyto_client(strdup("Enter month : "), csock);
			choice = 7;
		}
		else if (choice == 7){
			month = str_to_num(recvbuf);
			replyto_client(strdup("Enter year : "), csock);
			choice = 8;
		}
		else if (choice == 8){
			year = str_to_num(recvbuf);
			if (valid_date(day, month, year)){
				get_sys_date();
				printf("\n%d %d %d\n", sys_date, sys_month, sys_year);
				int var = check_date_in_6_months();
				if (var == 1){
					if (update_client_data())
						replyto_client(strdup("UnSuccessfull submition - client with that date is already registerd!!\n1.Back\n2.Exit"), csock);
					else
						replyto_client(strdup("Successfully submited!!\n1.Back\n2.Exit"), csock);
				}
				else if (var == 0){
					replyto_client(strdup("Date not in range of 6 months!!\n1.Back\n2.Exit"), csock);
				}
				else{
					replyto_client(strdup("U entered past date!!\n1.Back\n2.Exit"), csock);
				}
			}
			else{
				replyto_client(strdup("Invalid date!!\n1.Back\n2.Exit"), csock);
			}
			choice = 9;

		}
		else if (choice == 9){
			if (recvbuf[0] == '1'){
				get_data();
				int len = strlen(data);
				strcpy(data, strdup("\n========================Menu========================\n"));
				data = strcat(data, app);
				data = strcat(data, strdup("\n2. "));
				data = strcat(data, reg);
				data = strcat(data, strdup("\n3. Back\n4. Exit\nEnter : \n"));
				replyto_client(data, csock);
				data[len] = '\0';
				flag = 1;
				count = 1;
				choice = 0;
			}
			else{
				replyto_client(strdup("EXIT"), csock);
				exit(0);
			}
		}
	}

	else if (count == 2){
		if (choice == 0){
			if (recvbuf[0] == '1'){
				get_data();
				int len = strlen(data);
				strcat(data, strdup("\n========================Menu========================\nChoose : "));
				replyto_client(data, csock);
				data[len] = '\0';
				flag = 1;
				count = 1;
				choice = 4;
			}
			else if (recvbuf[0] == '2'){
				get_data();
				int len = strlen(data);
				strcat(data, strdup("\n========================Menu========================\nChoose : "));
				replyto_client(data, csock);
				data[len] = '\0';
				flag = 1;
				choice = 2;

			}
			else if (recvbuf[0] == '3'){

				strcpy(reply_buff, strdup("\n========================Menu========================\n"));
				reply_buff = strcat(reply_buff, app);
				reply_buff = strcat(reply_buff, strdup("\n2. "));
				reply_buff = strcat(reply_buff, reg);
				reply_buff = strcat(reply_buff, strdup("\n3. Back\n4. Exit\nEnter : \n"));
				replyto_client(reply_buff, csock);

				//replyto_client(strdup("\n========================Menu========================\n1. Appointment\n2. Register\n3. Back\n4. Exit\nEnter : \n"), csock);
				flag = 1;
				count = 1;
				choice = 0;
			}
			else{
				replyto_client(strdup("EXIT"), csock);
				exit(0);
			}
		}

		else if (choice == 1){
			if (recvbuf[0] == '1'){
				replyto_client(strdup("\n========================Menu========================\n1. Register as client\n2. View clients\n3. Back\n4. Exit\n"), csock);
				count = 2;
				choice = 0;
			}
			else{
				replyto_client(strdup("EXIT"), csock);
				exit(0);
			}
		}
		else if (choice == 2){

			choosen = str_to_num(recvbuf);
			int index = choosen;
			fseek(fp, block, SEEK_SET);
			int temp;
			if (index >= 0){
				fseek(fp, 4, SEEK_CUR);
			}
			while (index > 0){
				fseek(fp, 32 + 32 + 16, SEEK_CUR);
				fread(&temp, 4, 1, fp);
				fseek(fp, temp, SEEK_SET);
				index--;
			}
			fseek(fp, 32 + 32 + 16 + 4, SEEK_CUR);
			insert_pos = ftell(fp);

			view_clients();
			if (client_list != NULL){
				int len = strlen(client_list);
				strcat(client_list, strdup("\n========================Menu========================\n1. Back\n2. Exit\nEnter : \n"));
				replyto_client(client_list, csock);
			}
			else{
				replyto_client(strdup("No data found\n========================Menu========================\n1. Back\n2. Exit\nEnter : \n"), csock);
			}
			choice = 1;
		}
	}
	//replybuf[0] = '\0';
}

void replyto_client(char *buf, int *csock) {
	int bytecount;

	if ((bytecount = send(*csock, buf, strlen(buf) + 1, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free(csock);
	}
	printf("replied to client: %s\n", buf);
}

DWORD WINAPI SocketHandler(void* lp){
	int *csock = (int*)lp;

	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;

	memset(recvbuf, 0, recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free(csock);
		return 0;
	}

	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
	process_input(recvbuf, recv_byte_cnt, csock);

	return 0;
}
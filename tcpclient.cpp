#define _CRT_SECURE_NO_WARNINGS

 
#define WIN32_LEAN_AND_MEAN 
#include <windows.h> 
#include <winsock2.h> 
#include <ws2tcpip.h> // Директива линковщику: использовать библиотеку сокетов 
#pragma comment(lib, "ws2_32.lib") 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#define WEBHOST "127.0.0.1"

struct packed {
	char number[4];
	char day1;
	char month1;
	char year1[2];
	char day2;
	char month2;
	char year2[2];
	char hours;
	char mins;
	char secs;
	char *msg;
};
struct packed A[1000]; int i;

int init()
{
WSADATA wsa_data; 
return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data)); 
}

void deinit()
{
#ifdef _WIN32 // Для Windows следует вызвать WSACleanup в конце работы 
WSACleanup(); 
#else 
	// Для других ОС действий не требуется 
#endif 
}

int sock_err(const char* function, int s) 
{
int err;
#ifdef _WIN32 
err = WSAGetLastError();
#else
err = errno;
#endif
fprintf(stderr, "%s: socket error: %d\n", function, err); return -1;
}

		// Функция определяет IP-адрес узла по его имени. // Адрес возвращается в сетевом порядке байтов. 
unsigned int get_host_ipn(const char* name) 
{ 
	struct addrinfo* addr = 0; unsigned int ip4addr = 0;
		// Функция возвращает все адреса указанного хоста // в виде динамического однонаправленного списка 
	if (0 == getaddrinfo(name, 0, 0, &addr)) 
	{ 
		struct addrinfo* cur = addr; 
		while (cur) 
		{ // Интересует только IPv4 адрес, если их несколько - то первый 
			if (cur->ai_family == AF_INET) 
			{ 
				ip4addr = ((struct sockaddr_in*) cur->ai_addr)->sin_addr.s_addr;
				break; 
			}
			cur = cur->ai_next;
		}
	freeaddrinfo(addr);
	}
	return ip4addr;
}

void pars(char *mas, int ind)
{
	char data[3] = { 0 };
	data[0] = mas[0]; data[1] = mas[1];
	A[ind].day1 = atoi(data);
	data[0] = mas[3]; data[1] = mas[4];
	A[ind].month1 = atoi(data);
	char syear[4] = { mas[6], mas[7], mas[8], mas[9] };
	int year = htons(atoi(syear));
	memcpy(A[ind].year1, &year, 2);
	data[0] = mas[11]; data[1] = mas[12];
	A[ind].day2 = atoi(data);
	data[0] = mas[14]; data[1] = mas[15];
	A[ind].month2 = atoi(data);
	char syear1[4] = { mas[17], mas[18], mas[19], mas[20] };
	int year1 = htons(atoi(syear1));
	memcpy(A[ind].year2, &year1, 2);
	data[0] = mas[22]; data[1] = mas[23];
	A[ind].hours = atoi(data);
	data[0] = mas[25]; data[1] = mas[26];
	A[ind].mins = atoi(data);
	data[0] = mas[28]; data[1] = mas[29];
	A[ind].secs = atoi(data);

}

int pars_msgs(int s, char *file_name)
{
	FILE *msgs;
	msgs = fopen(file_name,"r");
	if (msgs == NULL)
	{
		printf("Error: file could't be opened\n");
		return -1;
	}
	int check_scan = 0;
	while(check_scan != -1)
	{
		char msg[32] = { 0 };
		fgets(msg, 31, msgs);
		if (msg[0] != '\n')
			check_scan = (int)fgetc(msgs);
		if (check_scan > 0 && msg[0] != '\0' && msg[0]!= '\n')
		{
			int k = htonl(i);
			memcpy(A[i].number, &k, 4);
			pars(msg, i);
			A[i].msg = (char *)malloc(1000000 * sizeof(char));
			memset(A[i].msg, 0, 1000000);
			//char ch = fgetc(msgs);
			fgets(A[i].msg, 1000000, msgs);
			int len = 0;
			while (A[i].msg[len] != '\n' && A[i].msg[len] != '\0')
				len++;
			if(A[i].msg[len] == '\n')
				A[i].msg[len] = '\0';
			i++;
		}
		
		//	fseek(msgs, -1, SEEK_CUR);
	}	
	fclose(msgs);
	return 0;
}

int send_recv(int s, int num_msgs)
{
	char ok[2]; int sending = 0;
	for(int j = 0; j < num_msgs; j++)
	{
		int len = 0;
		while (A[j].msg[len] != '\0')
			len++;
		memset(ok, '\0', 2);
		sending = send(s, A[j].number, 4, 0);
		if(sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].day1, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].month1, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, A[j].year1, 2, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].day2, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].month2, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, A[j].year2, 2, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].hours, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].mins, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, &A[j].secs, 1, 0);
		if (sending < 0)
			return sock_err("send", s);
		sending = send(s, A[j].msg, len+1, 0);
		if (sending < 0)
			return sock_err("send", s);

		int res = recv(s, ok, 2, 0);
		if (res < 0)
			return sock_err("recv", s);
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 3 && argc < 2)
	{
		printf("Need only 2 arguments\n");
		return 0;
	}
	char *argv1 = argv[1];
	char *file_name = argv[2];
	/*char argv1[20];
	char file_name[20];
	strcpy(argv1, "127.0.0.1:9000");
	strcpy(file_name, "msg.txt");*/
	
	char port[5] = { 0 }; int sms = 0, mms = 0;
	
	char ip_serv[10] = { 0 };
	while (argv1[sms] != ':')
	{
		ip_serv[sms] = argv1[sms];
		sms++;
	}
	sms++;
	while (argv1[sms] != '\0')
	{
		port[mms] = argv1[sms];
		sms++; mms++;
	}
	int p = atoi(port);
		// Инициалиазация сетевой библиотеки 
	//char *ip_serv = "127.0.0.1";
	//int p = 9000;
	//char *file_name = "cli1.txt";
	init();
	// Создание TCP-сокета  
	i = 0;
int s; struct sockaddr_in addr;

	s = socket(AF_INET, SOCK_STREAM, 0); 
	if (s < 0) 
		return sock_err("socket", s);
	// Заполнение структуры с адресом удаленного узла 
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; 
	addr.sin_port = htons(p); 
	addr.sin_addr.s_addr = get_host_ipn(ip_serv);
	bool con = false;
	// Установка соединения с удаленным хостом 
	for (int j = 0; j < 9; j++)
	{
		if (connect(s, (struct sockaddr*) &addr, sizeof(addr)) != 0)
			Sleep(100);
		else
		{
			con = true;
			break;
		}
	}
	if (con == false && connect(s, (struct sockaddr*) &addr, sizeof(addr)) != 0)
	{
		closesocket(s);
		return sock_err("connect", s);
	}
	else
	{
		char put = 112;
		int res = send(s, &put, 1, 0);
		put = 117;
		res = send(s, &put, 1, 0);
		put = 116;
		res = send(s, &put, 1, 0);
	}

	int check = pars_msgs(s, file_name);
	if (check == -1)
	{
		closesocket(s);
		deinit();
		return -1;
	}
	int num_msgs = i;
	int flag = send_recv(s, num_msgs);
	// Закрытие соединения 
	closesocket(s);
	for (int j = 0; j < i; j++)
		free(A[j].msg);
	deinit();
	if (flag == 0)
		return 0;
	else
		return -1;
}
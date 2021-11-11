#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32 
#define WIN32_LEAN_AND_MEAN 
#include <windows.h> 
#include <winsock2.h> 
#pragma comment(lib, "ws2_32.lib") 
#else // LINUX 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/time.h> 
#include <sys/select.h> 
#include <netdb.h> 
#include <errno.h> 
#endif
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#define N 6
int cs[N]; // Сокеты с подключенными клиентами

int init() 
{
#ifdef _WIN32 // Для Windows следует вызвать WSAStartup перед началом использования сокетов 
	WSADATA wsa_data; 
	return (0 == WSAStartup(MAKEWORD(2, 2), &wsa_data)); 
#else 
	return 1; // Для других ОС действий не требуется 
#endif 
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
fprintf(stderr, "%s: socket error: %d\n", function, err); 
return -1;
}

void s_close(int s)
{
#ifdef _WIN32 
closesocket(s); 
#else 
close(s); 
#endif 
}

int set_non_block_mode(int s) 
{ 
#ifdef _WIN32 
	unsigned long mode = 1; 
	return ioctlsocket(s, FIONBIO, &mode); 
#else 
	int fl = fcntl(s, F_GETFL, 0); 
	return fcntl(s, F_SETFL, fl | O_NONBLOCK); 
#endif 
}

void print_to_file(char *buf, unsigned int ip, int port, unsigned long *max, bool *start, bool *stop, int *num_msg_in_file)
{
	char num_msg[4] = { buf[0] ,  buf[1] , buf[2] , buf[3] }; unsigned long num;
	memcpy(&num, num_msg, 4);
	num = ntohl(num);
	*num_msg_in_file = num;
	char buf_msg[1000] = { 0 }; int ind_b = 15, ind_m = 0;
	while (buf[ind_b] != '\0')
	{
		buf_msg[ind_m] = buf[ind_b];
		ind_b++; ind_m++;
	}
	if (strcmp("stop", buf_msg) == 0)
		*stop = true;
	if (num <= *max && *start == false)
		return;
	*start = false;
	*max = num;
	FILE *f;
	f = fopen("msg.txt", "a");
	unsigned int first = (ip >> 24) & 0xFF, second = (ip >> 16) & 0xFF, theard = (ip >> 8) & 0xFF, fours = (ip) & 0xFF; int len, i = 0, j = 0, k = 0, m = 0, n = 0;
	char fir[4] = { 0 }, s[4] = { 0 }, th[4] = { 0 }, four[4] = { 0 }, pport[5] = { 0 }; char ip_port[16] = { 0 }; 
	_itoa(first, fir, 10); _itoa(second, s, 10); _itoa(theard, th, 10); _itoa(fours, four, 10);  _itoa(port, pport, 10);
	len = strlen(fir);
	while (fir[i] != '\0')
	{
		ip_port[i] = fir[i];
		i++;
	}
	ip_port[i] = '.'; i++;
	while (s[j] != '\0')
	{
		ip_port[i] = s[j];
		i++; j++;
	}
	ip_port[i] = '.'; i++;
	while (th[k] != '\0')
	{
		ip_port[i] = th[k];
		i++; k++;
	}
	ip_port[i] = '.'; i++;
	while (four[m] != '\0')
	{
		ip_port[i] = four[m];
		i++; m++;
	}
	ip_port[i] = ':'; i++;
	while (pport[n] != '\0')
	{
		ip_port[i] = pport[n];
		i++; n++;
	}
	ip_port[i] = ' ';
	fprintf(f, "%s", ip_port);
	
	char s_day[3]; int day = buf[4];
	if (day < 10)
		fprintf(f, "0%i.", day);
	else
		fprintf(f, "%i.", day);
	day = buf[5]; 
	if (day < 10)
		fprintf(f, "0%i.", day);
	else
		fprintf(f, "%i.", day);
	char s_year[2] = { buf[6], buf[7] }; unsigned short year;
	memcpy(&year, s_year, 2);
	year = ntohs(year);
	fprintf(f, "%i ", year);
	day = buf[8];
	if (day < 10)
		fprintf(f, "0%i.", day);
	else
		fprintf(f, "%i.", day);
	day = buf[9];
	if (day < 10)
		fprintf(f, "0%i.", day);
	else
		fprintf(f, "%i.", day);
	s_year[0] = buf[10]; s_year[1] = buf[11];
	memcpy(&year, s_year, 2);
	year = ntohs(year);
	fprintf(f, "%i ", year);
	day = buf[12];
	if (day < 10)
		fprintf(f, "0%i:", day);
	else
		fprintf(f, "%i:", day);
	day = buf[13];
	if (day < 10)
		fprintf(f, "0%i:", day);
	else
		fprintf(f, "%i:", day);
	day = buf[14];
	if (day < 10)
		fprintf(f, "0%i ", day);
	else
		fprintf(f, "%i s", day);
	
	fprintf(f, "%s\n", buf_msg);
	fclose(f);
	
}

int main(int argc, char **argv) 
{

	int ports[N];
	
	unsigned long max = 0;
	bool start = true, stop = false;
	int flags = 0; 

	// Инициалиазация сетевой библиотеки
	init();
	char *p1 = argv[1];
	char *p2 = argv[2];
	// Создание UDP-сокета
	int port2 = atoi(p2), port1 = atoi(p1);
	struct sockaddr_in addr;
	for (int i = 0; i <= (port2 - port1); i++)
	{
		cs[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if (cs[i] < 0)
			return sock_err("socket", cs[i]);
		//Заполняем структуру адреса
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		ports[i] = port1 + i;
		addr.sin_port = htons(ports[i]); // Будет прослушиваться порт 8000 
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(cs[i], (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
			return sock_err("bind", cs[i]);
		}
		set_non_block_mode(cs[i]);
	}

	int nfds = cs[0]; 
	int i;  bool flag = false;
	struct timeval tv = { 10, 10 };
	int con = 0; 
	char buffer[100000] = { 0 }; int len = 0; int answer;
	fd_set rfd;
	fd_set wfd;
	while (1)
	{
		//Обнулили структуры
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		for (int i = 0; i <= port2-port1; i++)
		{
			FD_SET(cs[i], &rfd);
			FD_SET(cs[i], &wfd);
			if (nfds < cs[i])
				nfds = cs[i];
		}
		if (select(nfds + 1, &rfd, &wfd, 0, &tv) > 0)
		{
			for (int i = 0; i <= port2 - port1; i++)
			{
				if (FD_ISSET(cs[i], &rfd))
				{
					//Вызвал ресив, принял структуру, первые 4 байта обязательно запомнить и отправить его в ответ, сообщение напечатал в файл
					memset(buffer, 0, 100000);
					int addrlen = sizeof(addr);
					int recv = recvfrom(cs[i], buffer, sizeof(buffer), flags, (struct sockaddr*) &addr, &addrlen);
					unsigned int ip = ntohl(addr.sin_addr.s_addr);
					print_to_file(buffer, ip, ports[i], &max, &start, &stop, &answer);
					char sm[4] = { 0 };
					answer = htonl(answer);
					memcpy(sm, &answer, 4);
					int send_len = sendto(cs[i], sm, 4, flags, (struct sockaddr*) &addr, addrlen);
					if (stop == true)
						break;
				}
			}
		}
	if (stop == true)
		break;
	}
	for (int j = 0; j < N; j++)
		s_close(cs[j]);
	deinit();
	return 0; 
}
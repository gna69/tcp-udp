#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <poll.h>

#define N 50

int cs[N]; // Сокеты с подключенными клиентами
bool check = true;
int init()
{
  return 1; // Для других ОС действий не требуется
}

int sock_err(const char* function, int s)
{
  int err;
  err = errno;
  fprintf(stderr, "%s: socket error: %d\n", function, err);
  return -1;
}

void s_close(int s)
{
  close(s);
}

bool check_put(int cs)
{
char put[4] = { 0 };
int len = recv(cs,put,3,MSG_NOSIGNAL);
for(int k = 0; k < 3; k++)
{
  int ch = put[k];
  if (k == 0 && ch != 112)
   return false;
 
  if (k == 1 && ch != 117)
   return false;
  if (k == 2 && ch != 116)
   return false;
}
check = false;
return true;
}

bool print_to_file(int s, int ip, int port)
{
FILE *f;
if(check == true)
  if(check_put(s) == false)
	  return false;
f = fopen("msg.txt", "a");
char num_msg[4]; int Nmsg;
int len = recv(s,num_msg,4,MSG_NOSIGNAL);
memcpy(&Nmsg, num_msg, 4);
if(len<=0)
return false;


fprintf(f, "%u.%u.%u.%u:", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip) & 0xFF);
fprintf(f, "%i ", port);

char day;int d;
len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);
len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);

char year[2]; int k;
len = recv(s,year,2,MSG_NOSIGNAL);
memcpy(&k, year, 2);
k = htons(k);
fprintf(f, "%i ", k);

len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);
len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);

len = recv(s,year,2,MSG_NOSIGNAL);
memcpy(&k, year, 2);
k = htons(k);
fprintf(f, "%i ", k);

len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);
len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);
len = recv(s,&day,1,MSG_NOSIGNAL);
d = day;
if(d < 10)
	fprintf(f, "0%i.", day);
else
	fprintf(f, "%i.", day);

char msg[500000] = { 0 };
len = recv(s,msg,sizeof(msg)+1,MSG_NOSIGNAL);
int msg_len = strlen(msg);
fprintf(f, "%s\n", msg);

send(s, "ok",2, MSG_NOSIGNAL);
if(!strcmp(msg, "stop"))
{
  for(int i = 0; i<N; i++)
    s_close(cs[i]);
  exit(0);
}
fclose(f);
return true;
}



int main(int argc, char *argv[])
{

if(argc > 2 )
{
	printf("Need only 1 argument\n");
	return 0;
}

int ls;
char *port = argv[1];
int p = atoi(port);


struct pollfd pfd[N+1];
int i;

struct sockaddr_in addr;
// Создание TCP-сокета
ls = socket(AF_INET, SOCK_STREAM, 0);
if (ls < 0)
    return sock_err("socket", ls);
// Заполнение адреса прослушивания
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(p); // Сервер прослушивает порт 9000
addr.sin_addr.s_addr = htonl(INADDR_ANY); // Все адреса
// Связывание сокета и адреса прослушивания
if (bind(ls, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    return sock_err("bind", ls);
// Начало прослушивания
if (listen(ls, 1) < 0)
    return sock_err("listen", ls);

unsigned int addrlen = sizeof(addr);

unsigned int ip;

// В отличие от select, массив pfd не обязательно заполнять перед каждым вызовом poll
for (i = 0; i < N; i++)
{
	pfd[i].fd = cs[i];
	pfd[i].events = POLLIN | POLLOUT;
}

pfd[N].fd = ls;
pfd[N].events = POLLIN;
int con = 0;
while (1)
{
// Ожидание событий в течение 1 сек
int ev_cnt = poll(pfd, sizeof(pfd) / sizeof(pfd[0]), 1000);

	if (ev_cnt > 0)
	{
		if (pfd[N].revents & POLLIN)
		{
      		int h;
     		socklen_t addrlen = sizeof(addr);
      		do {
 				h = accept(ls, (struct sockaddr*)&addr, &addrlen);	
     			if(h>0)
					printf("Accepting client: %d\n", con);
      			cs[con] = h;
				pfd[con].fd = h;
      	}while(h<0);
      	con++; 
		}
		for (i = 0; i < N; i++)
		{
			if (pfd[i].revents & POLLHUP)
			{
				// Сокет cs[i] - клиент отключился. Можно закрывать сокет
				s_close(cs[i]);
			}
			if (pfd[i].revents & POLLERR)
			{
				// Сокет cs[i] - возникла ошибка. Можно закрывать сокет
				s_close(cs[i]);	
			}
			if (pfd[i].revents & POLLIN)
			{
				// Сокет cs[i] доступен на чтение, можно вызывать recv/recvfrom
        		bool er = true;
				ip = ntohl(addr.sin_addr.s_addr);
         		do{
				 	er = print_to_file(cs[i], ip, p);
        	 	}while(er == true);
       		  	printf("Disconnected socket\n");
      		  	check = true;
       		  	s_close(cs[i]);
			}
		}	
	}
	else
	{
		// Тайтмаут или ошибка
	}
}

return 0;
}

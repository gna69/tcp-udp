#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <stdlib.h>

// Функция извлекает IPv4-адрес из DNS-дейтаграммы.
// Задание л/р не требует детального изучения кода этой функции

char *msgs[20] = { 0 }; int lens_msgs[20]; bool send_m = false; int msg_in_datagram = 0;

unsigned int get_addr_from_dns_datagram(const char* datagram, int size);

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

void send_request(int s, struct sockaddr_in* addr, char *msg, int len)
{

int res = sendto(s, msg, len, MSG_NOSIGNAL, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
printf ("sending bytes is %i\n", res);
if (res <= 0)
	sock_err("sendto", s);
}

// Функция принимает дейтаграмму от удаленной стороны.
// Возвращает 0, если в течение 100 миллисекунд не было получено ни одной дейтаграммы

int recv_response(int s)
{
char datagram[1024];
struct timeval tv = {0, 100*1000}; // 100 msecint res;
fd_set fds;
FD_ZERO(&fds);
FD_SET(s, &fds);

// Проверка - если в сокете входящие дейтаграммы
// (ожидание в течение tv)
int res = select(s + 1, &fds, 0, 0, &tv);
if (res > 0)
{

	// Данные есть, считывание их
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int received = recvfrom(s, datagram, sizeof(datagram), 0, (struct sockaddr*) &addr, &addrlen);
	printf("Receive bytes is %i\n", received);
	printf("There are is:\n");
	for(int m = 0; m < received; m++)
		printf("%i ", datagram[m]);
	printf("\n");
	if (received <= 0)
	{
		// Ошибка считывания полученной дейтаграммы
		sock_err("recvfrom", s);
		return 0;
	}
	return 0;
}
else 
	if (res == 0)
	{
		// Данных в сокете нет, возврат ошибки
		return 0;
	}
	else
	{
		sock_err("select", s);
		return 0;
	}
}

bool send_msg(int s, FILE *f, int nms, int *flag_end, bool *no_inc)
{
	char ch;char day, year[2], num_msg[4], work[3] = { 0 }; char defoult[16] = { 0 };
			int for_day, for_year, for_num_msg;
	nms = ntohl(nms); memcpy(num_msg, &nms, 4);
	defoult[0] = num_msg[0];defoult[1] = num_msg[1];defoult[2] = num_msg[2];defoult[3] = num_msg[3];//4 bytes for nomber of msg in daitagram
	char prov = fgetc(f);
	if (prov == '\n')
	{
		*no_inc = true;
		return false;
	}
	if(prov == EOF)
	{
		*flag_end = 1;
		return true;
	}
	work[0] = prov;work[1] = fgetc(f);
	day = atoi(work);
	defoult[4] = day;//1 byte of day in datagram
	ch = fgetc(f);//dot
	work[0] = fgetc(f);work[1] = fgetc(f);
	day = atoi(work);
	defoult[5] = day;//1 byte of mounse in datagram
	ch = fgetc(f);//dot
	num_msg[0] = fgetc(f);num_msg[1] = fgetc(f);num_msg[2] = fgetc(f);num_msg[3] = fgetc(f);
	for_year = atoi(num_msg); for_year = ntohs(for_year); memcpy(year, &for_year, 2);
	defoult[6] = year[0];defoult[7] = year[1];// 2 bytes of year in datagram
	ch = fgetc(f);//space
	work[0] = fgetc(f);work[1] = fgetc(f);
	day = atoi(work);
	ch = fgetc(f);//dot	
	defoult[8] = day;// 1 byte of day in dategram
	work[0] = fgetc(f);work[1] = fgetc(f);
	day = atoi(work);
	defoult[9] = day;// 1 byte of mounse in datagram
	ch = fgetc(f);//dot	
	num_msg[0] = fgetc(f);num_msg[1] = fgetc(f);num_msg[2] = fgetc(f);num_msg[3] = fgetc(f);
	for_year = atoi(num_msg); for_year = ntohs(for_year); memcpy(year, &for_year, 2);
	defoult[10] = year[0];defoult[11] = year[1];// 2 bytes of year in datagram
	ch = fgetc(f);//space
	work[0] = fgetc(f);work[1] = fgetc(f);
	day = atoi(work);
	ch = fgetc(f);//double dot	
	defoult[12] = day;// hour in datagram
	work[0] = fgetc(f);work[1] = fgetc(f);
	day = atoi(work);
	ch = fgetc(f);//double dot	
	defoult[13] = day;// minuts on datagram
	work[0] = fgetc(f);work[1] = fgetc(f);
	day = atoi(work);
	ch = fgetc(f);//space	
	defoult[14] = day;// seconds in datagram
	ch = fgetc(f);//first letter of message
	char buf[500000] = { 0 }; int i = 0; bool flag = false;
	while(ch != '\n')
	{
		buf[i] = ch;
		ch = fgetc(f);
		if(ch == EOF)
		{
			flag = true;
			break;
		}
		i++;
	}
	int len = strlen(buf);
	len+=15;
	lens_msgs[msg_in_datagram] = len+1;
	printf("Len is %i\n", len);
	msgs[msg_in_datagram] = (char *)malloc((len+1)*sizeof(char));
	memset(msgs[msg_in_datagram], 0, (len+1)*sizeof(char));
	memcpy(msgs[msg_in_datagram], defoult, sizeof(defoult));
	int j = 15; i = 0;
	while(buf[i] != '\0')
	{
		msgs[msg_in_datagram][j] = buf[i];
		i++;j++;
	}
	for (int puk = 0; puk < j; puk++)
	 printf("%i ",msgs[msg_in_datagram][puk]);
	if(msg_in_datagram + 1 == 20)
		return true;
	msg_in_datagram++;
	if(flag == true)
		return true;
	return false;
}

int main(int argc, char **argv)
{

if(argc > 3  && argc < 2)
{
	printf("Need only 2 argument\n");
	return 0;
}

char *file_name = argv[2]; bool no_inc = false;
char *sub = argv[1];
char ip[20] = { 0 }, port [5] = { 0 };
int sms = 0, mms = 0;
while(sub[sms]!= ':')
{
ip[sms] = sub[sms];
sms++; 
}
sms++;
while(sub[sms]!= '\0')
{
port[mms] = sub[sms];
sms++; mms++;
}
printf("%s %s %s\n", file_name, ip, port);
int p = atoi(port);

FILE *f = fopen(file_name, "r");
if(f == NULL)
{
	printf("Error: file could not open\n");
	return 0;
}
int s;
struct sockaddr_in addr;
int i;

// Инициалиазация сетевой библиотеки
init();

// Создание UDP-сокета
s = socket(AF_INET, SOCK_DGRAM, 0);
if (s < 0)
	return sock_err("socket", s);

// Заполнение структуры с адресом удаленного узла
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(p); // Порт DNS - 53
addr.sin_addr.s_addr = inet_addr(ip);

// Выполняется 5 попыток отправки и затем получения дейтаграммы.
// Если запрос или ответ будет потерян - данные будут запрошены повторно
int n = 0;int flag_end = 0;
while(flag_end != 1)
{
send_m = false;	msg_in_datagram = 0;
while(send_m == false)
{
send_m = send_msg(s, f, n, &flag_end, &no_inc);
if(no_inc != true)
	n++;
else
	no_inc = false;
}

for(int k = 0; k < 20; k++)
{
	if(msgs[k] != NULL)
	{
	printf("Messege nomber %i is sending\n", k+1);
	send_request(s, &addr, msgs[k], lens_msgs[k]);
	recv_response(s);
	}
}

for(int k = 0; k < 20; k++)
{
	if(msgs[k] != NULL)
	{
		free(msgs[k]);
		msgs[k] = NULL;
	}
}
}
/*for (i = 0; i < 5; i++)
{
	printf(" sending request: attempt %d\n", i + 1);
	// Отправка запроса на удаленный сервер	
	send_request(s, &addr);
	// Попытка получить ответ. Если ответ получен - завершение цикла попыток
	if (recv_response(s))
	{
		break;
	}
}*/

// Закрытие сокета

fclose(f);
s_close(s);
return 0;
}

unsigned int get_addr_from_dns_datagram(const char* datagram, int size)
{
unsigned short req_cnt, ans_cnt, i;
const char* ptr;
req_cnt = ntohs(*(unsigned short*)(datagram + 4));
ans_cnt = ntohs(*(unsigned short*)(datagram + 6));
ptr = datagram + 12;
for (i = 0; i < req_cnt; i++)
{
unsigned char psz;
do
{
psz = *ptr;
ptr += psz + 1;
} while (psz > 0);
ptr += 4;
}
for (i = 0; i < ans_cnt; i++)
{
unsigned char psz;
unsigned short asz;
do
{
psz = *ptr;
if (psz & 0xC0)
{
ptr += 2;
break;
}
ptr += psz + 1;
} while (psz > 0);
ptr += 8;
asz = ntohs(*(unsigned short*)ptr);
if (asz == 4)
{
printf(" Found IP: %u.%u.%u.%u\n",
(unsigned char)ptr[1], (unsigned char)ptr[2], (unsigned char)ptr[3],
(unsigned char)ptr[4]);
}
ptr += 2 + asz;}
return 1;
}


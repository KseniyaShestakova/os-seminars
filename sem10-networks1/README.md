# Сети


## Модель OSI
Разделяет современные сетевые протоколы на 7 уровней:
* **Physical layer** - передача данных через физическую среду: Bluetooth, Ethernet
* **Link layer** - общение компьютеров в локальной сети (LAN - local area network): Ethernet, IEEE 802.11
* **Network layer** - связь между разными LAN: Internet Protocol (IPv4 и IPv6)
* **Transport layer** - обмен пакетами данных между приложениями: TCP, UDP, SCTP
* **Session layer** - поддержание концепта сессии между приложениями: Zone Information Protocol (ZIP), Session Control Protocol (SCP)
* **Presentation layer** - представление данных, приведение их в читаемый для конечного приложения вид, форматирование: eXternal Data Representation (XDR), Network Data Representation (NDR)
* **Application layer** - обмен данными между приложениями: HyperText Transport Protocol (HTTP), HTTPS, Simple Mail Transfer Protocol (SMTP)

На текущий момент такая классификация немного устарела, т.к. некоторые сетевые протоколы оперируют одновременно на нескольких уровнях. Например, протоколы TLS (Transport Layer Security) и SSL (Secure Socket Layer) работают на 6 и 7 уровнях.

## Сокеты
Материал частично заимствован из [ридинга Яковлева](https://github.com/victor-yacovlev/fpmi-caos/blob/master/practice/sockets-tcp/README.md?plain=1), в нем можно подробнее почитать про сокеты и системные вызовы для работы с ними.

Сокет - это файловый дескриптор, открытый как для чтения, так и для записи. Предназначен для взаимодействия:
 * разных процессов, работающих на одном компьютере (*хосте*);
 * разных процессов, работающих на разных *хостах*.

Создается сокет с помощью системного вызова `socket`:

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int socket(
  int domain,    // тип пространства имён
  int type,      // тип взаимодействия через сокет
  int protocol   // номер протокола или 0 для авто-выбора
)
```

Актуальные типы пространства имен:
 * `AF_UNIX` (`man 7 unix`) - пространство имен локальных UNIX-сокетов, которые позволяют взаимодействовать разным процессам в пределах одного компьютера, используя в качестве адреса уникальное имя (длиной не более 107 байт) специального файла.
 * `AF_INET` (`man 7 ip`) - пространство кортежей, состоящих из 32-битных IPv4 адресов и 16-битных номеров портов. IP-адрес определяет хост, на котором запущен процесс для взаимодействия, а номер порта связан с конкретным процессом на хосте.
 * `AF_INET6` (`man 7 ipv6`) - аналогично `AF_INET`, но используется 128-разрядная адресация хостов IPv6; пока этот стандарт поддерживается не всеми хостерами и провайдерами сети Интернет.
 * `AF_PACKET` (`man 7 packet`) - взаимодействие на низком уровне.

Через сокеты обычно происходит взаимодействие одним из двух способов (указывается в качестве второго параметра `type`):
 * `SOCK_STREAM` - взаимодействие с помощью системных вызовов `read` и `write` как с обычным файловым дескриптором. В случае взаимодействия по сети, здесь подразумевается использование протокола `TCP`.
 * `SOCK_DGRAM` - взаимодейтсвие без предвариательной установки взаимодействия для отправки коротких сообщений. В случае взаимодействия по сети, здесь подразумевается использование протокола `UDP`.

## Пара сокетов

Иногда сокеты удобно использовать в качестве механизма взаимодействия между разными потоками или родственными процессами: в отличии от каналов, они являются двусторонними, и кроме того, поддерживают обработку события "закрытие соединения". Пара сокетов создается с помощью системного вызова `socketpair`:

```
int socketpair(
  int domain,    // В Linux поддерживатся только AF_UNIX
  int type,      // SOCK_STREAM или SOCK_DGRAM
  int protocol,  // Только значение 0 в Linux
  int sv[2]      // По аналогии с pipe, массив из двух int
)
```

Сокеты могут выполнять одну из двух ролей - *клиент* или *сервер*.

Для того, чтобы сокет был готов к системным вызовам `read` или `write`, нужно установить соединение, что делается с помощью системного вызова `connect`:
```cpp
int connect(
  int sockfd,                  // файловый дескриптор сокета

  const struct sockaddr *addr, // указатель на *абстрактную*
                               // структуру, описывающую
                               // адрес подключения

  socklen_t addrlen            // размер реальной структуры,
                               // которая передается в
                               // качестве второго параметра
)
```

Для использования сокета в роли сервера, необходимо выполнить следующие действия:

 1. Связать сокет с некоторым адресом. Для этого используется системный вызов `bind`, параметры которого точно такие же, как для системного вызова `connect`. Если на компьютере более одного IP-адреса, то адрес `0.0.0.0` означает "все адреса". Часто при отладке и возникает проблема, что порт с определенным номером уже был занят на предыдущем запуске программы (и, например, не был корректно закрыт). Это решается принудительным повторным использованием адреса:

```
// В релизной сборке такого обычно быть не должно!
#ifdef DEBUG
int val = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
#endif
```

 2. Создать очередь, в которой будут находиться входящие, но ещё не принятые подключения. Это делается с помощью системного вызова `listen`, который принимает в качестве параметра максимальное количество ожидающих подключений. Для Linux это значение равно 128, определено в константе `SOMAXCONN`.

 3. Принимать по одному соединению с помощью системного вызова `accept`. Второй и третий параметры этого системного вызова могуть быть `NULL`, если нас не интересует адрес того, кто к нам подключился. Системный вызов `accept` блокирует выполнение до тех пор, пока не появится входящее подключение. После чего - возвращает файловый дескриптор нового сокета, который связан с конкретным клиентом, который к нам подключился.
 
 ## getaddrinfo
 Этот системный вызов используется для того, чтобы по *node* (хосту) и *service* (сервису) получить структуру `addrinfo`, которая содержит адрес, который позже можно передать в вызовы `bind` и `connect`.
 
```cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *restrict node,
               const char *restrict service,
               const struct addrinfo *restrict hints,
               struct addrinfo **restrict res);
```


```python
%%file getaddrinfo.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <sys/syscall.h>
#include <time.h>
#include <stdatomic.h>
#include <stdbool.h>


// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 19;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


int try_connect_by_name(const char* name, int port, int ai_family) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    size_t len;
    ssize_t nread;
   
    /* Obtain address(es) matching host/port */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = ai_family;    
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    
    char port_s[20];
    sprintf(port_s, "%d", port);
    s = getaddrinfo(name, port_s, &hints, &result);
    if (s != 0) {
        log_printf("getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if (getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
            log_printf("Try ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            log_printf("Success with ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
            break;                  /* Success */
        }
        close(sfd);
    }

    freeaddrinfo(result);
    
    if (rp == NULL) {               /* No address succeeded */
        log_printf("Could not connect to %s:%d\n", name, port);
        return -1;
    }
    return sfd;
}


int main() { 
    try_connect_by_name("localhost", 22, AF_UNSPEC);
    try_connect_by_name("localhost", 22, AF_INET);
    try_connect_by_name("localhost", 22, AF_INET6);
    try_connect_by_name("ya.ru", 80, AF_UNSPEC);
    try_connect_by_name("ya.ru", 80, AF_INET6);
    return 0;
}

```

    Writing getaddrinfo.c



```python
%cat socket_tcp.c
```

    #include <error.h>
    #include <errno.h>
    #include <netinet/in.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <netdb.h>
    #include <string.h>
    #include <stdbool.h>
    
    void output_without_header(char* buff, size_t sz, FILE* stream) {
    	bool headers_received = false;
    	bool n_found = false;
    	while (fgets(buff, sz, stream) != NULL) {
    		if (headers_received) {
    			printf("%s", buff);
    			continue;
    		}
    
    		int len = strlen(buff);
    		if (n_found && len >= 1 && buff[0] == '\n') {
    			printf("%s", (char*)buff + 1);
    			headers_received = true;
    			continue;
    		}
    		if (n_found && len >= 2 && buff[0] == '\r' && buff[1] == '\n') {
    			printf("%s", (char*)buff + 2);
    			headers_received = true;
    			continue;
    		}
    		char* pos = strstr(buff, "\n\r\n");
    		if (pos != NULL) {
    			headers_received = true;
    			printf("%s", pos + 3);
    			continue;
    		}
    		pos = strstr(buff, "\n\n");
    		if (pos != NULL) {
    			headers_received = true;
    			printf("%s", pos + 2);
    			continue;
    		}
    		n_found =  (buff[len - 1] == '\n');
    	}
    }
    
    
    int main(int argc, char* argv[]) {
      const char* hostname = argv[1];
      const char* filepath = argv[2];
    
      const struct addrinfo addr_hints = {
          .ai_family = AF_INET,
          .ai_socktype = SOCK_STREAM
      };
      struct addrinfo* res_list;
      if (getaddrinfo(hostname, /*service=*/"http", &addr_hints, &res_list) != 0) {
        fprintf(stderr, "Could not obtain server address");
        return EXIT_FAILURE;
      }
      const struct addrinfo* first_res = res_list;
    
      int server_sock = socket(AF_INET, SOCK_STREAM, /*protocol=*/0);
      int connect_status =
          connect(server_sock, first_res->ai_addr, first_res->ai_addrlen);
      freeaddrinfo(res_list);
      if (connect_status == -1) {
        error(EXIT_FAILURE, errno, "Could not connect to the server");
      }
    
      const size_t request_size = 1024;
      char* request_buff = (char*) calloc(request_size, 1);
      const char* request_fmt =
          "GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "Connection: close\r\n\r\n";
      snprintf(request_buff, request_size, request_fmt, filepath, hostname);
      ssize_t
          send_status = send(server_sock, request_buff, request_size, /*flags=*/0);
      free(request_buff);
      if (send_status == -1) {
        error(EXIT_FAILURE, errno, "Could not send request to the server");
      }
    
      const size_t response_size = 8;
      char* response_buff = (char*) calloc(response_size, 1);
      FILE* sock_stream = fdopen(server_sock, /*mode=*/"r");
      
      output_without_header(response_buff, response_size, sock_stream);
      
      free(response_buff);
    
      fclose(sock_stream);
    }


Попробуем добыть данные с сайта [example.com](https://example.com/)


```bash
%%bash
gcc socket_tcp.c -o socket_tcp
./socket_tcp example.com /
```

    <!doctype html>
    <html>
    <head>
        <title>Example Domain</title>
    
        <meta charset="utf-8" />
        <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <style type="text/css">
        body {
            background-color: #f0f0f2;
            margin: 0;
            padding: 0;
            font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", "Open Sans", "Helvetica Neue", Helvetica, Arial, sans-serif;
            
        }
        div {
            width: 600px;
            margin: 5em auto;
            padding: 2em;
            background-color: #fdfdff;
            border-radius: 0.5em;
            box-shadow: 2px 3px 7px 2px rgba(0,0,0,0.02);
        }
        a:link, a:visited {
            color: #38488f;
            text-decoration: none;
        }
        @media (max-width: 700px) {
            div {
                margin: 0 auto;
                width: auto;
            }
        }
        </style>    
    </head>
    
    <body>
    <div>
        <h1>Example Domain</h1>
        <p>This domain is for use in illustrative examples in documents. You may use this
        domain in literature without prior coordination or asking for permission.</p>
        <p><a href="https://www.iana.org/domains/example">More information...</a></p>
    </div>
    </body>
    </html>


## Системный вызов bind

При создании сокета с помощью `socket`, он появляется в пространстве имен, но при этом с ним не связан никакой адрес. `bind` называет сокету, связанному с файловым дескриптором *sockfd*, адрес *addr*. Для того, чтобы сокет мог работать как сервер, т.е. принимать входящие соединения, необходимо связать его с локальным адресом через `bind`.
```cpp
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr,
        socklen_t addrlen);
```

## Системный вызов listen
Используется для того, чтобы указать, что сокет, связанный с файловым дескриптором *sockfd*, будет принимать водящие соединения с помощью `accept`, т.е. фактически будет сервером. *backlog* - указывает максимальный размер очереди входящих соединений для *sockfd*.

```cpp
#include <sys/socket.h>

int listen(int sockfd, int backlog);
```

## Системный вызов accept
Используется для того, чтобы принимать входящие соединения.

```cpp
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *_Nullable restrict addr,
          socklen_t *_Nullable restrict addrlen);

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/socket.h>

int accept4(int sockfd, struct sockaddr *_Nullable restrict addr,
          socklen_t *_Nullable restrict addrlen, int flags);
```

## Псевдокод с примером использования
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define MY_SOCK_PATH "/somepath"
#define LISTEN_BACKLOG 50

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

int
main(void)
{
   int                 sfd, cfd;
   socklen_t           peer_addr_size;
   struct sockaddr_un  my_addr, peer_addr;

   sfd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sfd == -1)
       handle_error("socket");

   memset(&my_addr, 0, sizeof(my_addr));
   my_addr.sun_family = AF_UNIX;
   strncpy(my_addr.sun_path, MY_SOCK_PATH,
           sizeof(my_addr.sun_path) - 1);

   if (bind(sfd, (struct sockaddr *) &my_addr,
            sizeof(my_addr)) == -1)
       handle_error("bind");

   if (listen(sfd, LISTEN_BACKLOG) == -1)
       handle_error("listen");

   /* Now we can accept incoming connections one
      at a time using accept(2). */

   peer_addr_size = sizeof(peer_addr);
   cfd = accept(sfd, (struct sockaddr *) &peer_addr,
                &peer_addr_size);
   if (cfd == -1)
       handle_error("accept");

   /* Code to deal with incoming connection(s)... */

   if (close(sfd) == -1)
       handle_error("close");

   if (unlink(MY_SOCK_PATH) == -1)
       handle_error("unlink");
}
```

## TCP vs. UDP 
В предыдущих примерах мы рассматривали соединения по протоколу транспортного уровня TCP (SOCK_STREAM). Альтернативой ему является протокол UDP. 

Протокол TCP требует установки соединения (хэндшейк), поэтому после создания сокета типа `SOCK_STREAM` нужно либо подключиться к противоположной стороне с помощью системного вызова `connect`, либо принять входящее подключение с помощью системного выхова `accept`. Сетевое взаимодействие по TCP/IP (создание сокета с параметрами `AF_INET` и `SOCK_STREAM`) подразумевает, что ядро операционной системы преобразует непрерывный поток данных в последовательность TCP-сегментов, упакованных в IP-пакеты, и наоборот.

Механизм отправки сообщений по UDP подразумевает передачу данных без предварительной установки соединения. Сокет, ориентированный на отправку UDP-сообщений имеет тип `SOCK_DGRAM` и используется совместно с адресацией IPv4 (`AF_INET`) либо IPv6 (`AF_INET6`).

```c
// Создание сокета для работы по UDP/IP
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
```

Как и в случае с TCP, для адресация UDP подразумевает, что помимо IP-адреса хоста необходимо определиться с номером порта, который обслуживает отдельный процесс.

### Системные вызовы для передачи и приема данных без установки соединения

```c
// Отправить пакет данных
ssize_t sendto(int sockfd,                  // сокет
               const void *buf, size_t len, // данные и размер
               int flags,                   // дополнительные опции
               // адрес назначения (и его размер как для bind/connect)
               const struct sockaddr *dest_addr, socklen_t addrlen);

// Получить пакет данных
ssize_t recvfrom(int sockfd,             // сокет
                 void *buf, size_t len,  // данные и размер
                 int flags,              // дополнительные опции
                 // адрес отправителя (и размер как для accept)
                 const struct sockaddr *src_addr, socklen_t *addrlen);               
```

Cистемный вызов `sendto` предназначен для отправки сообщения. Поскольку предварительно соединение не было установлено, то обязательным является указание адреса назначения: IP-адрес хоста и номер порта.

Системный вызов `recvfrom` предназначен для приема сообщения, и является блокирующим до тех пор, пока придет хотя бы одно сообщение UDP.

Размер буфера, в который `recvfrom` должен записать данные, должен быть достаточного размера для хранения сообщения, в противном случае данные, которые не влезли в буфер, будут потеряны.

Для того, чтобы иметь возможность принимать данные по UDP, необходимо анонсировать прослушивание определенного порта с помощью системного вызова `bind`; параметры адреса для `recvfrom` предназначены только для получения информации об отправителе, и являются опциональными (эти значения могут быть NULL).


## Утилиты для работы с сетями

### tcpdump
Установка:
```
apt install tcpdump
```

* `tcpdump -D` - вывести список доступных к прослушиванию интерфейсов
* `tcpdump -i <interface>` - захват пакетов, проходящих через `interface`
* `tcpdump -c 5` - захватить только первые 5 строк
* `tcpdump -tttt` - захват с указанием красивых временных меток

### dig
**DNS (Domain Name System)** - сервис для преобразования человекочитаемых доменных имен в IP-адреса. 

Для того, что бы получить информацию из DNS, можно воспользоваться утилитой `dig`. Пример: `dig google.com`.

Альтернатива утилите `dig` - `nslookup`. Пример: `nslookup google.com`.

### whois
Сетевой протокол на основе TCP, используемый для того, чтобы узнавать информацию о регистрируемых именах. Обращается к специальной базе данных со всеми зарегистрированными именами.

Примеры:
`whois google.com / yandex.ru / example.com`

### curl
Утилита для работы с сетями. Примеры использования:
* `curl https://www.example.com/` - получить главную страницу сайта
* `curl http://www.example.com:8000/` - получить страницу сервера на порту 8000

Подробнее о доступных опциях - `man curl`.

У `curl` есть API на C.

### whireshark
Прилодение для мониторинга трафика, проходящего через ваше устройство. Поддерживает различные фильтры пакетов.


```python

```

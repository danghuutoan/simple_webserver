/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *header = 
"HTTP/1.1 200 OK\r\n"
"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
"Server: Apache/2.2.14 (Win32)\r\n"
"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
"Content-Length: 100\r\n"
"Content-Type: text/html\r\n"
"Connection: Closed\r\n";

char *html_txt =
"<!DOCTYPE HTML>\r\n" 
"<html>\r\n"
"<body>\r\n"
"<h1>Hello, World!</h1>\r\n"
"</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>\r\n"
"</body>\r\n"
"</html>\r\n";
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int web_socket_read(char **buffer, int fd)
{
	char *temp;
	int  received_bytes = 0;
	int  n;
	int size = 0;
	printf("%s %d\n", __FUNCTION__, __LINE__);
	n = 256;
	*buffer = NULL;

	while (n >= 256)
	{
		*buffer = (char*)realloc (*buffer, sizeof(char)*(size += 256));
		temp = *buffer + received_bytes;
		n = recv(fd, temp, 256, 0);
		received_bytes += n;
		printf("received_bytes %d\n", received_bytes);
		if (n < 0) error("ERROR reading from socket");
	}
	return received_bytes;
}

/* copy from http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
	thank you a lots for creating such a great tutorial */
int socket_init(char *portno)
{
	struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    // !! don't forget your error checking for these calls !!

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, portno, &hints, &res);

    // make a socket, bind it, and listen on it:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, 10);

	return sockfd;

}

void http_parser(char *request_str, char **method, char **uri, char **http_version)
{
	*method = strtok (request_str," ");
	*uri = strtok (NULL," ");
	*http_version = strtok (NULL," ");
}
int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char *buffer = NULL;
	int recv_len = 0;
	int request_len = 0;
	char *request_str;
	char *ptr;
	char *request_http_version;
	char *request_method;
	char *request_uri;

	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}

	sockfd = socket_init(argv[1]);

	clilen = sizeof(cli_addr);
	while(1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) 
		  error("ERROR on accept");

		printf("waiting for request\n");
		recv_len = web_socket_read(&buffer, newsockfd);

		/* searching for the first \r\n string */
  		ptr = strstr(buffer,"\r\n");
  		request_len = ptr - buffer ;
  		request_str = malloc(sizeof(char)*request_len +1);
		memcpy(request_str, buffer, request_len);
  		request_str[request_len] = 0;

  		http_parser(request_str, &request_method, &request_uri, 
  			&request_http_version);

		printf("%s\n", request_method);
		printf("%s\n", request_uri);
		printf("%s\n", request_http_version);	
	
		free(request_str);
		free(buffer);
		send(newsockfd, html_txt, strlen(html_txt), 0);
	}

	close(newsockfd);
	close(sockfd);
	return 0; 
}
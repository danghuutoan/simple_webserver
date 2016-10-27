/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


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

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char *buffer = NULL;
	int recv_len = 0;
	int request_len = 0;
	char *request_str;
	char *ptr;
	char *request_protocol;
	char *request_cmd;
	char *request_data;

	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
	      sizeof(serv_addr)) < 0) 
	      error("ERROR on binding");

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while(1)
	{
		newsockfd = accept(sockfd, 
	         (struct sockaddr *) &cli_addr, 
	         &clilen);

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

		request_cmd = strtok (request_str," ");

		if(strcmp(request_cmd,"GET") == 0)
		{
			printf("%s\n", request_cmd);
			request_data = strtok (NULL," ");
			printf("%s\n", request_data);
			request_protocol = strtok (NULL," ");
			printf("%s\n", request_protocol);	
		}
		free(request_str);
		free(buffer);
		send(newsockfd, html_txt, strlen(html_txt), 0);
	}

	close(newsockfd);
	close(sockfd);
	return 0; 
}
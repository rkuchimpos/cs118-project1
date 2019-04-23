#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORTNO 33623
#define BACKLOG 10

void handle_request(int fd) {
	const int buf_size = 8192;
	char buf[buf_size];

	ssize_t n = recv(fd, buf, buf_size - 1, 0);
	if (n < 0) {
		perror("recv error");
		return;
	}

	printf("REQUEST:\n");
	printf("%s", buf);
}

void get_content_type(char *filename, char *content_type) {
	char *extension = strchr(filename, '.');

	if (extension == NULL) {
		/* TODO: handle case when extension is unspecified */
	}

	if (strcmp(extension, ".html") == 0) {
		strcpy(content_type, "text/html");
	} else if (strcmp(extension, ".html") == 0) {
		strcpy(content_type , "text/html");
	} else if (strcmp(extension, ".txt") == 0) {
		strcpy(content_type, "text/plain");
	} else if (strcmp(extension, ".jpg") == 0) {
		strcpy(content_type, "image/jpeg");
	} else if (strcmp(extension, ".jpeg") == 0) {
		strcpy(content_type, "image/jpeg");
	} else if (strcmp(extension, ".png") == 0) {
		strcpy(content_type, "image/png");
	} else if (strcmp(extension, ".gif") == 0) {
		strcpy(content_type, "image/gif");
	} else {
		/* TODO: figure out what to do here */
	}
}


int main() {
	int sockfd; /* listen on sock_fd */
	int new_fd; /* new connection on new_fd */
	struct sockaddr_in server_addr; /* my address */
	struct sockaddr_in client_addr; /* connector addr */
	socklen_t sin_size;

 	/* create socket */

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Failed to create socket!\n");
		exit(-1);
	}

	/* fill fields of sockaddr_in */

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(PORTNO);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* should not depend on htonl */

	/* bind socket */

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
		perror("Failed to bind socket!\n");
		exit(-1);
	}

	/* listen for connections on the socket */

	if (listen(sockfd, BACKLOG) == -1) {
		perror("Failed to listen!\n");
		exit(-1);
	}


	/* accept connections */

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		if (new_fd == -1) {
			perror("Unable to accept connection");
			continue;
		}

		handle_request(new_fd);
		close(new_fd);
	}

}
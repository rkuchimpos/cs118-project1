#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>

#define PORTNO 33623
#define BACKLOG 10

#define FILENAME_SIZE 256

#define ERROR_404_PAGE "error_404.html"

long int get_file_size(FILE *f) {
	fseek(f, 0, SEEK_END);
	long int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return size;
}

void decode_space(char *str, char *decoded) {
	int len = strlen(str);
	int i = 0;
	int j = 0;
	while (i < len) {
		if (i < len - 3 && str[i] == '%' && str[i + 1] == '2' && str[i + 2] == '0') {
			decoded[j] = ' ';
			i += 3;
			j++;
		} else {
			decoded[j] = str[i];
			i++;
			j++;
		}
	}
	decoded[j] = '\0';
}

/*
	* if we have extension, wext is same string
	* else, wext is filename with extension (wext)
		* wext is filename if no matches

	returns 0 upon 200 success, 1 upon 404 error code
*/
int match_filename(char *filename, char *wext) {
	char *extension = strrchr(filename, '.');
	char temp_wext[FILENAME_SIZE];

	DIR *curr_dir;
	struct dirent *dir;

	if (extension != NULL)
		strcpy(temp_wext, filename);

	curr_dir = opendir(".");
	if (curr_dir) {
		while ((dir = readdir(curr_dir)) != NULL) {
			strcpy(temp_wext, filename);

			if (extension == NULL) {
				/* get next file in directory extension */
				char *dir_extension = strrchr(dir->d_name, '.');

				/* add extension to filename if directory file has one */
				if (dir_extension != NULL) {
					for (int i = 0; i < strlen(dir_extension); i++)
						dir_extension[i] = tolower(dir_extension[i]);

					/* create filename with extension */
					strcat(temp_wext, dir_extension);
				}
			}

			/* case insensitivity */
			for (int i = 0; i < strlen(dir->d_name); i++)
				dir->d_name[i] = tolower(dir->d_name[i]);

			if (strcmp(temp_wext, dir->d_name) == 0) {
				strcpy(wext, temp_wext);
				closedir(curr_dir);
				return 0;
			}
		}

		/* no such file in current directory */
		strcpy(wext, filename);
		closedir(curr_dir);
		return -1;
	}

	strcpy(wext, filename);
	return -1;
}

void parse_request(char *req, char *filename) {
	/* break request into first line */
	char *line;
	line = strtok(req, "\n");

	int len = strlen(line);
	int filename_start = 3;
	int filename_end = len - 1 - 9;
	while (filename_start < len && req[filename_start] == ' ')
		filename_start++;
	filename_start++; /* Consume forward slash */
	while (filename_end >= 0 && req[filename_end] == ' ')
		filename_end--;
	int filename_len = filename_end - filename_start + 1;
	char extracted_filename[filename_len + 1];
	memcpy(extracted_filename, &req[filename_start], filename_len);
	extracted_filename[filename_len] = '\0';
	//strcpy(filename, extracted_filename);
	decode_space(extracted_filename, filename);
	//printf("filename: %s\n", extracted_filename);
}

void send_response(int fd, char *header, char *body, long int content_size) {
	write(fd, header, strlen(header));
	write(fd, "\n", 1);
	write(fd, body, content_size);
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

void handle_request(int fd) {
	const int request_size = 8192;
	char request[request_size];
	char header[2048];
	char filename[FILENAME_SIZE];
	char filename_wext[FILENAME_SIZE];
	long int file_size = 0;
	FILE *f;

	ssize_t n = recv(fd, request, request_size - 1, 0);
	if (n < 0) {
		perror("recv error");
		return;
	}
	request[n] = '\0';

	printf("%s", request);

	parse_request(request, filename);

	int TEST = match_filename(filename, filename_wext);

	printf("%d\n", TEST);

	int ret = match_filename(filename, filename_wext);
	// DEBUG ----------------
	if (ret == -1) {
		if ((f = fopen(ERROR_404_PAGE, "rb")) == NULL) {
			/* handle error */
		}
		file_size = get_file_size(f);
		sprintf(header, "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: %ld\n", file_size);
	} else {
		/* open the file for binary reading */
		if ((f = fopen(filename_wext, "rb")) == NULL) {
			/* handle error */
		}
		file_size = get_file_size(f);
		char content_type[16];
		get_content_type(filename_wext, content_type);
		sprintf(header, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %ld\n", content_type, file_size);
	}
	char *body = malloc(file_size);
	fread(body, file_size, 1, f);
	send_response(fd, header, body, file_size);

	fclose(f);
	free(body);
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
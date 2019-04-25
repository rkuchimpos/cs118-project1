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

#define PORTNO 33624
#define BACKLOG 10

#define FILENAME_SIZE 256

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
        	if (extension == NULL) {
        		strcpy(temp_wext, filename);

        		/* get next file in directory extension */
        		char *dir_extension = strrchr(dir->d_name, '.');
        		for (int i = 0; i < strlen(dir_extension); i++)
					dir_extension[i] = tolower(dir_extension[i]);

        		/* create filename with extension */
        		strcat(temp_wext, dir_extension);
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
        return 1;
    }

    strcpy(wext, filename);
    return 1;
}

void parse_request(char *buf, char *filename) {
	/* break request into first line */
	char *line;
	line = strtok(buf, "\n");

	/* retrieve file name */
	char *field;
	field = strtok(line, "/");
	field = strtok(NULL, " ");

	for (int i = 0; i < strlen(field); i++)
		field[i] = tolower(field[i]);

	/* copy filename to return value */
	strcpy(filename, field);
}

void handle_request(int fd) {
	const int buf_size = 8192;
	char buf[buf_size];
	char filename[FILENAME_SIZE];
	char filename_wext[FILENAME_SIZE];
	FILE *f;

	ssize_t n = recv(fd, buf, buf_size - 1, 0);
	if (n < 0) {
		perror("recv error");
		return;
	}

	printf("%s", buf);

	parse_request(buf, filename);

	int test = match_filename(filename, filename_wext);

	printf("%d\n", test);

	/* open the file for binary reading */
	if ((f = fopen(filename, "rb")) == NULL) {
		/* handle error */
	}

	/* do whatever with f */

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
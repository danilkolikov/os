#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

void error_happened() {
	char* error_buf = strerror(errno);
	printf("%s", error_buf);
}
	
int main(int argc, char** args) {
	char buf[1024];
	int read_len, write_len;
	int read_fd = 0;
	
	if (argc == 2) {
		read_fd = open(args[1], O_RDONLY);
		if (read_fd == -1) {
			error_happened();
			return errno;
		}
	}

	while (1) {
		read_len = read(read_fd, buf, 1024);
		if (read_len == 0) {
			break;
		}
		
		if (read_len == -1) {
			if (errno == EINTR) {
				continue;
			}
			error_happened();
			return errno;
		}

		write_len = 0;
		int cur_write;
		while (1) {
			cur_write = write(1, buf + write_len, read_len - write_len);
			if (cur_write == -1) {
				if (errno == EINTR) {
					continue;
				}
				error_happened();
				return errno;
			}
			
			write_len += cur_write;
			if (write_len == read_len) {
				break;
			}
		}
	}
	
	if (argc == 2) {
		close(read_fd);
	}


	return 0;
}	 

#include <unistd.h>
#include <string.h>
#include <errno.h>

void error_happened() {
	char* error_buf = strerror(errno);
	write(2, error_buf, strlen(error_buf));
}
	
int main() {
	char buf[1024];
	int read_len, write_len;
	
	while ((read_len = read(0, buf, 1024)) > 0) {
		write_len = 0;
		int cur_write;
		while ((cur_write = write(1, buf + write_len, read_len - write_len)) != -1) {
			write_len += cur_write;
			if (write_len == read_len) {
				break;
			}
		}
		if (cur_write == -1) {
			// Error happened
			error_happened();
			return errno;
		}
	}
	if (read_len == -1) {
		error_happened();
		return errno;
	}
	return 0;
}	 

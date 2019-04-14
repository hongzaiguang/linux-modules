#include <stdio.h>
#include <fcntl.h>

const char docker_dev_name[] = "/dev/docker_dev";

int main(int argc, const char *argv[])
{
	int fd;
	
	fd = open(docker_dev_name, O_RDWR);
	if (fd < 0) {
		printf("open %s failed\n", docker_dev_name);
		return -1;
	}
	close(fd);
	
	sleep(100);
	return 0;
}

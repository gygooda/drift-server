#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int check_single(const char* fpath)
{
    if(fpath == NULL)
        return -1;

    int fd = open(fpath, O_RDWR, 0644);
    if (fd < 0)
        return -1;

    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = getpid();

    if (fcntl(fd, F_SETLK, &fl) < 0) 
    {
        if (errno == EACCES || errno == EAGAIN) 
        {
            close(fd);
            return 1;
        }

        close(fd);
        return 2;
    }

    close(fd);
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Usage: \n\t%s <file-name>\n", argv[0]);
        exit(3);
    }

    if(access(argv[1], F_OK) == 0)
    {
        int ret = check_single(argv[1]);
        if(ret == 1 || ret == 0)
            exit(ret);
        else
            exit(2); // error
    }

    // the file not exist
    return 0;
}

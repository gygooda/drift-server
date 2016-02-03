#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "one_way_channel.h"
#include "dlog.h"

OneWayChannel::OneWayChannel()
{
    m_pipe_fds[0] = -1;
    m_pipe_fds[1] = -1;
}

OneWayChannel::~OneWayChannel()
{
    this->close();
}

int OneWayChannel::open()
{
    int ret = pipe2(m_pipe_fds, O_NONBLOCK);
    if(ret != 0)
    {
        m_pipe_fds[0] = -1;
        m_pipe_fds[1] = -1;
        ERROR_LOG("call pipe2() failed: %s.", strerror(errno)); // thread safe: strerror_r()
    }
    return ret;
}

void OneWayChannel::close()
{
    ::close(recv_end());
    ::close(send_end());
}

int OneWayChannel::send(const char* buf, uint32_t len)
{
    return ::write(send_end(), buf, len);
}

int OneWayChannel::recv(char* buf, uint32_t len)
{
    return ::read(recv_end(), buf, len);
}

int OneWayChannel::register_send_end(int epoll_fd)
{
}

int OneWayChannel::register_recv_end(int epoll_fd)
{
}

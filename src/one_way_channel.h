#pragma once

#include "common.h"
#include "event_handler.h"

/**
 * 单向通道，数据只能往一个方向传输。
 * 内部使用pipe实现，读写数据大小小于PIPE_BUF，
 * 可保证原子性。
 */
class OneWayChannel
{
public:
    OneWayChannel();
    virtual ~OneWayChannel();

    int open();
    void close();

    int send(const char* buf, uint32_t len);
    int recv(char* buf, uint32_t len);

    // 将发送端注册到epoll中监听
    int register_send_end(int epoll_fd); 
    // 将接收端注册到epoll中监听
    int register_send_end(int epoll_fd); 

private:
    int send_end()
    {
        return m_pipe_fds[1];
    }
    int recv_end()
    {
        return m_pipe_fds[0];
    }

private:
    int m_pipe_fds[2];
};

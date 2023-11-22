/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-11-22 10:12:39
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-22 17:42:55
 * @FilePath: /cpp-library/C++高性能服务器/test/test_iomanager.cpp
 * @Description: 
 * 
 * Copyright (c) 2023 by ${git_name_email}, All Rights Reserved. 
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include "../atpdxy/atpdxy.h"
#include "../atpdxy/iomanager/iomanager.h"

atpdxy::Logger::ptr g_logger=ATPDXY_LOG_ROOT();

int sock = 0;

void test_fiber() {
    ATPDXY_LOG_INFO(g_logger) << "teststopping_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //ATPDXY::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        ATPDXY_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        atpdxy::IOManager::GetThis()->addEvent(sock, atpdxy::IOManager::READ, [](){
            ATPDXY_LOG_INFO(g_logger) << "read callback";
        });
        atpdxy::IOManager::GetThis()->addEvent(sock, atpdxy::IOManager::WRITE, [](){
            ATPDXY_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            atpdxy::IOManager::GetThis()->cancelEvent(sock, atpdxy::IOManager::READ);
            close(sock);
        });
    } else {
        ATPDXY_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    atpdxy::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

void test()
{
    atpdxy::IOManager iom;
    iom.schedule(&test_fiber);
}

int main()
{
    test1();
    return 0;
}

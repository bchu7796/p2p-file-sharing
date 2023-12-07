/**
MIT License

Copyright (c) 2019 Pang-Yang Chu (Brian Chu) and contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**/

#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>

#include <thread>   
#include <mutex>

#define CHUNK_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

struct peer{
    uint32_t address;
    uint16_t port;
    std::vector<uint32_t> chunks;
};

struct file_info{
    std::string file_name;
    std::uint32_t file_length;
    std::vector<peer> peers;
};
/*
int calculate_chunk_number(uint32_t) – Input file length and get how many chunks are needed with this file length.
int register_reply(int, struct sockaddr_in) – Reply to the register request from a peer and put peer’s and files’ information into a data structure.
int file_list_reply(int) – Reply to the file list request from a peer by giving a list of files available.
int location_reply(int) – Reply to the file location request from a peer by giving a list of peers having the file requested by the peer.
int chunk_register_reply(int, struct sockaddr_in) – Reply to the chunk register request from a peer and put peer’s chunk information into a data structure.
void handle_request(int, uint32_t, struct sockaddr_in) – Get the request type of the message and call related functions.
*/

class server{
    private:
        int caculate_chunk_number(uint32_t file_length);
        int register_reply(int process_fd, struct sockaddr_in client_address);
        int file_list_reply(int process_fd);
        int location_reply(int process_fd);
        int chunk_register_reply(int process_fd, struct sockaddr_in client_address);
        void handle_request(int new_sock, uint32_t request_type, struct sockaddr_in client_address);
    public:
        int execute(void);
};
#endif

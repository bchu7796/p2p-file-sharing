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

#include "server.hpp"

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

std::vector<file_info> files;
std::mutex mtx;

int server::caculate_chunk_number(uint32_t file_length){
    int chunk_number = file_length / CHUNK_SIZE;
    if((chunk_number * CHUNK_SIZE) < file_length){
        chunk_number += 1;
    }
    return chunk_number;
}

int server::register_reply(int process_fd, struct sockaddr_in client_address){
    uint16_t number_of_file;
    std::string file_name;
    uint32_t file_length;
    std::vector<file_info> temp_files = files;
    std::string response = "registered";

    //client's address info
    std::cout<<"client:"<< inet_ntoa(client_address.sin_addr)<<":"<<client_address.sin_port<<std::endl;
    uint16_t port;
    if(recv(process_fd, &port, sizeof(port), 0) < 0){
        perror("client connection failed");
        return -1;
    }

    if(recv(process_fd, &number_of_file, sizeof(number_of_file), 0) < 0){
        perror("client connection failed");
        return -1;
    }

    for(int i=0; i<number_of_file; i++){
        if(recv(process_fd, &file_name, sizeof(file_name), 0) < 0){
            perror("client connection failed");
            return -1;
        }
        if(recv(process_fd, &file_length, sizeof(file_length), 0)<0){
            perror("client connection failed");
            return -1;
        }
        std::cout<<"file name:"<<file_name<<", "<<"file length:"<<file_length<<"\n";

        //register for all the chunks
        struct peer temp_peer;
        temp_peer.address = client_address.sin_addr.s_addr;
        temp_peer.port = port;
        int chunk_size = caculate_chunk_number(file_length);
        
        for(int j=0; j< chunk_size; j++){
            temp_peer.chunks.push_back(j);
        }
        //fill in other data
        struct file_info temp_file_info;
        temp_file_info.file_name = file_name;
        temp_file_info.file_length = file_length;
        temp_file_info.peers.push_back(temp_peer);
        temp_files.push_back(temp_file_info);
    }
    mtx.lock();
    files = temp_files;
    mtx.unlock();

    send(process_fd, &response, sizeof(response), 0);

    close(process_fd);

    return 0;
}

int server::file_list_reply(int process_fd){
    uint16_t number_of_file = files.size();
    send(process_fd, &number_of_file, sizeof(number_of_file), 0);
    for(int i=0; i<number_of_file; i++){
        send(process_fd, &files[i].file_name, sizeof(files[i].file_name), 0);
        send(process_fd, &files[i].file_length, sizeof(files[i].file_length), 0);
    }
    close(process_fd);
    return 0;
}

int server::location_reply(int process_fd){
    std::string file_name;
    recv(process_fd, &file_name, sizeof(file_name), 0);
    int i;
    for(i = 0; i < files.size(); i++){
        if(files[i].file_name == file_name){
            uint16_t number_of_address = files[i].peers.size();
            send(process_fd, &number_of_address, sizeof(number_of_address), 0);
            send(process_fd, &files[i].file_length, sizeof(files[i].file_length), 0);
            for(int j=0; j<number_of_address; j++){
                send(process_fd, &files[i].peers[j].address, sizeof(files[i].peers[j].address), 0);
                send(process_fd, &files[i].peers[j].port, sizeof(files[i].peers[j].port), 0);
                uint16_t number_of_chunks = files[i].peers[j].chunks.size();
                send(process_fd, &number_of_chunks, sizeof(number_of_chunks), 0);
                for(int k = 0; k < number_of_chunks; k++){
                    send(process_fd, &files[i].peers[j].chunks[k], sizeof(files[i].peers[j].chunks[k]), 0);
                }
            }
        }
    }
    if( i == files.size()){
        uint16_t number_of_address = 0;
        send(process_fd, &number_of_address, sizeof(number_of_address), 0);
    }
    close(process_fd);
    return 0;
}

int server::chunk_register_reply(int process_fd, struct sockaddr_in client_address){
    std::string file_name;
    int index;
    uint16_t port;
    std::string response = "registered";

    if(recv(process_fd, &file_name, sizeof(file_name), 0) < 0){
        perror("client connection failed");
        return -1;
    }

    if(recv(process_fd, &index, sizeof(index), 0) < 0){
        perror("client connection failed");
        return -1;
    }

    if(recv(process_fd, &port, sizeof(port), 0) < 0){
        perror("client connection failed");
        return -1;
    }

    std::vector<file_info> temp_files = files;
    for(int i=0; i<temp_files.size(); i++){
        if(file_name == temp_files[i].file_name){
            std::cout<<"hello"<<std::endl;
            int j = 0;
            for(j=0; j<temp_files[i].peers.size(); j++){
                if(temp_files[i].peers[j].address == client_address.sin_addr.s_addr && temp_files[i].peers[j].port == port){
                    temp_files[i].peers[j].chunks.push_back(index);
                    break;
                }
            }
            if (j == temp_files[i].peers.size()){
                struct peer temp_peer;
                temp_peer.address = client_address.sin_addr.s_addr;
                temp_peer.port = port;
                temp_peer.chunks.push_back(index);
                temp_files[i].peers.push_back(temp_peer);
                break;
            }
        }
    }
    mtx.lock();
    files = temp_files;
    mtx.unlock();

    send(process_fd, &response, sizeof(response), 0);
    close(process_fd);

    return 0;
}

void server::handle_request(int new_sock, uint32_t request_type, struct sockaddr_in client_address){
    switch (request_type){
        case 0:
            register_reply(new_sock, client_address);
            break;
        case 1:
            file_list_reply(new_sock);
            break;
        case 2:
            location_reply(new_sock);
            break;
        case 3:
            chunk_register_reply(new_sock, client_address);
            break;
        default:
            break;
    }
    return;
}

int server::execute(void){
    server myserver;
    int process_fd;
    int server_fd;
    struct sockaddr_in server_address, client_address;
    //create socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Failed to create socket"); 
        exit(0);
    }

    //initailize address structure
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = PF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons(SERVER_PORT); 

    //bind socket
    if(bind(server_fd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        perror("Failed to bind"); 
        exit(0);
    }

    //listen socket
    if(listen(server_fd, 5) < 0){
        perror("Failed to listen"); 
        exit(0);
    }


    while(1){
        int client_address_length = sizeof(client_address);
        if((process_fd = accept(server_fd,(struct sockaddr*) &client_address, (socklen_t *) &client_address_length)) < 0){
            perror("Failed to accept"); 
            exit(0);
        }
        uint32_t request_type;
        recv(process_fd, &request_type, sizeof(request_type), 0);
        //use multi-thread to handle requests
        std::thread tid(&server::handle_request, myserver, process_fd, request_type, client_address);
        tid.detach();
        //handle_request(process_fd, request_type, client_address);
    }
    return 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <strings.h>

#include <thread>   
#include <mutex>

#define PORT 8081
#define CHUNK_SIZE 1024

/*
	int calculate_chunk_number(uint32_t) – Input file length and get how many chunks are needed with this file length.
	int register_request(uint16_t, std::vector<file_info>) – Request server to register shared files.
	int file_list_request() – Request server to give a list of available files.
	int location_request(std::string, std::vector<chunk>&, int &) – Request server to give a list of location where the file is at.
	void split(const std::string &, std::vector<string>&, const char) – Split the given sting using a specific delimiter.
	file_exist(std::string) – Check if a file exists.
	int write_log(std::string, std::vector<chunk>) – Write a log file for recording the status.
	int read_log(std::string, std::vector<chunk>, int) – Read a log file to resume the status of previous section.
	std::vector<int> select_chunk(std::vector<chunk>) – Choose which chunk to download first.
	int select_peer(int) – Choose which peer to download from.
	int download_file(std::string, int, struct chunk) – Download a chunk of a file from a peer.
	int combine_file(std::string, int ) – Combine all the chunks downloaded.
	int register_chunk(std::string, int) – Register possessing a chunk.
	int download(std::string) – Download a file.
	long get_file_size(std::string) – Get the size of a file.
	int parse_command(std::string) – Parse a command entered by users.
	int user_interface() – User interface.
	void handle_request(int, std::string, int) – Handle download request from other peers.
*/

std::mutex mtx;

struct sockaddr_in server_address;

struct file_info{
    std::string file_name;
    uint32_t file_length;
};

struct peer{
    in_addr address;
    uint16_t port;
};

struct chunk{
    uint32_t index;
    bool is_possessed;
    std::vector<peer> peers;
};

int calculate_chunk_number(uint32_t file_length){
    int chunk_size = file_length / CHUNK_SIZE;
    if((file_length * chunk_size) < file_length){
        chunk_size += 1;
    }

    return chunk_size;
}

int register_request(uint16_t number_of_file, std::vector<file_info> file_infos){
    /*
    send:
        request type
        the port number a peer is listening to
        number of file a peer want to share
        a list of file name
        a list of file length
    receive:
        OK
    */
   int peer_fd;
    char receiveMessage[100] = {};
    uint32_t file_length = 0;
    std::string file_name;
    const uint32_t request_type = 0;
    uint16_t port = PORT;
    std::string response_message;

    //create socket
    if((peer_fd = socket(AF_INET, SOCK_STREAM , 0)) < 0){
        perror("Failed to create socket"); 
        exit(0);
    }

    //connect to server
    if(connect(peer_fd,(struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        perror("Failed to connect"); 
        exit(0);
    }
    
    //register the file a peer want to share
    send(peer_fd, &request_type, sizeof(request_type), 0);
    send(peer_fd, &port, sizeof(port), 0);
    send(peer_fd, &number_of_file, sizeof(number_of_file), 0);
    for(int i=0; i < number_of_file; i++){
        send(peer_fd, &file_infos[i].file_name, sizeof(file_infos[i].file_name), 0);
        send(peer_fd, &file_infos[i].file_length, sizeof(file_infos[i].file_length), 0);
    }
    if(recv(peer_fd, &response_message, sizeof(response_message), 0) < 0){
        perror("server response failed");
        return -1;
    }
    std::cout << "server response:" << response_message << std::endl;
    close(peer_fd);
    return 0;
}

int file_list_request(){
    /*
    send:
        request type
    receive:
        number of file a client can download
        a list of file name
        a list of file length
    */
    int peer_fd;
    const uint32_t request_type = 1;
    uint16_t number_of_file;
    std::string file_name;
    uint32_t file_length;

    //create socket
    if((peer_fd = socket(AF_INET, SOCK_STREAM , 0)) < 0){
        perror("Failed to create socket"); 
        exit(0);
    }

    //connect to server
    if(connect(peer_fd,(struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        perror("Failed to connect"); 
        exit(0);
    }
    //get files a client can download
    send(peer_fd, &request_type, sizeof(request_type), 0);
    recv(peer_fd, &number_of_file, sizeof(number_of_file), 0);
    for(int i = 0; i < number_of_file; i++){
        recv(peer_fd, &file_name, sizeof(file_name), 0);
        recv(peer_fd, &file_length, sizeof(file_length), 0);
        std::cout << file_name << ": "<< file_length <<" bytes";
        if(i != number_of_file - 1){
            std::cout<<", ";
        }
    }
    std::cout << std::endl;
    close(peer_fd);

    return 0;
}

int location_request(std::string file_name, std::vector<chunk>& chunks, int & file_size){
    /*
    send:
        request type
        file name
    receive:
        number_of_location
        file_length
        a list of ip_address
        a list of port
        a list of number_of_chunks
        a list of chunk index for every ip_address

    */
    int peer_fd;
    const uint32_t request_type = 2;
    uint16_t number_of_location;
    uint16_t number_of_chunk;
    uint32_t ip_address;
    uint16_t port;
    struct in_addr temp_address;
    uint32_t chunk_index;
    uint32_t file_length;
    //std::vector<chunk> chunks;
    struct chunk temp_chunk;
    struct peer temp_peer;

    //create socket
    if((peer_fd = socket(AF_INET, SOCK_STREAM , 0)) < 0){
        perror("Failed to create socket"); 
        exit(0);
    }

    //connect to server
    if(connect(peer_fd,(struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        perror("Failed to connect"); 
        exit(0);
    }

    //ask for list of addresses having the file
    send(peer_fd, &request_type, sizeof(request_type), 0);
    send(peer_fd, &file_name, sizeof(file_name), 0);
    recv(peer_fd, &number_of_location, sizeof(number_of_location), 0);
    
    //file is not found
    if(number_of_location == 0){
        std::cout<<"file not found"<<std::endl;
        return -1;
    }
    //create data structure to store file location info
    recv(peer_fd, &file_length, sizeof(file_length), 0);
    file_size = file_length;
    int chunk_number = calculate_chunk_number(file_length);
    for(int i = 0; i<chunk_number; i++){
        temp_chunk.index = i;
        temp_chunk.is_possessed = false;
        chunks.push_back(temp_chunk);
    }

   //store the routing information into client's data structure.
    for(int i=0; i<number_of_location; i++){
        recv(peer_fd, &ip_address, sizeof(ip_address), 0);
        recv(peer_fd, &port, sizeof(port), 0);
        temp_address.s_addr = ip_address;

        recv(peer_fd, &number_of_chunk, sizeof(number_of_chunk), 0);
        for(int j=0; j<number_of_chunk; j++){
            recv(peer_fd, &chunk_index, sizeof(chunk_index), 0);
            for(int k = 0; k<chunks.size(); k++){
                if(chunks[k].index == chunk_index){
                    temp_peer.address.s_addr = ip_address;
                    temp_peer.port = port;
                    chunks[k].peers.push_back(temp_peer);
                }
            }
        }
    }
    close(peer_fd);
    return 0;
}

void split(const std::string& s, std::vector<std::string>& parameters, const char delim = ' ') {
    parameters.clear();
    std::istringstream iss(s);
    std::string temp;

    while (std::getline(iss, temp, delim)) {
        parameters.emplace_back(std::move(temp));
    }
    return;
}

bool file_exist(std::string file_name){
    struct stat buffer;   
    return (stat (file_name.c_str(), &buffer) == 0); 
}

int write_log(std::string file_name, std::vector<chunk> chunks){
    std::ofstream log;
    log.open((file_name + ".log").c_str());
    for(int i = 0; i < chunks.size(); i++){
        if(chunks[i].is_possessed == true){
            log<<chunks[i].index<<"\n";
        }
    }
    log.close();
    return 0;
}

int read_log(std::string file_name, std::vector<chunk>& chunks, int file_length){
    std::ifstream log;
    std::ofstream download_file;
    char buffer[10];
    std::string log_line;
    std::vector<std::string> elements;

    
    if(!file_exist(file_name + ".log")){
        /*
        download_file.open(file_name);
        for(int i=0; i < file_length; i++){
            download_file << "0";
        }
        download_file.close();
        */
        return 0;
    }
    

    log.open((file_name + ".log").c_str());
    
    while(log.getline(buffer, 10)){
        log_line = buffer;
        std::stringstream s(log_line);
        int index = 0;
        s >> index;
        chunks[index].is_possessed = true;
        std::stringstream().swap(s);
    }
    return 0;
}

std::vector<int> select_chunk(std::vector<chunk> chunks){
    std::vector<int> min;
    int min_number = INT_MAX;
    int min_index = 0;
    bool flag = false;
    for(int i = 0; i<4; i++){
        for(int j=0; j<chunks.size(); j++){
            if((chunks[j].peers.size() < min_number) && (!chunks[j].is_possessed)){
                min_number = chunks[j].peers.size();
                min_index = j;
                flag = true;
            }
        }
        if(flag){
            min.push_back(min_index);
            chunks[min_index].is_possessed = true;
            min_number = INT_MAX;
            flag = false;
        }
    }
    return min;
}

int download_file(std::string file_name, int index, struct chunk chunk_instance){
    int sock_fd;
    char buffer[1024];
    struct sockaddr_in peer_address;
    int selected_peer_index = rand() % chunk_instance.peers.size();

    
    //initialize address structure
    bzero(&peer_address, sizeof(peer_address));
    peer_address.sin_family = PF_INET;
    peer_address.sin_addr.s_addr = chunk_instance.peers[selected_peer_index].address.s_addr;
    peer_address.sin_port = htons(chunk_instance.peers[selected_peer_index].port);
    std::cout<<"LIST"<<std::endl;
    std::cout<<"peer: "<< inet_ntoa(chunk_instance.peers[selected_peer_index].address)\
    << ":" << chunk_instance.peers[selected_peer_index].port << " chunk: " << index << std::endl; 

    //create socket
    if((sock_fd = socket(AF_INET, SOCK_STREAM , 0)) < 0){
        perror("Failed to create socket"); 
        exit(0);
    }

    //connect to server
    if(connect(sock_fd,(struct sockaddr*) &peer_address, sizeof(peer_address)) < 0){
        perror("Failed to connect"); 
        exit(0);
    }

    send(sock_fd, &file_name, sizeof(file_name), 0);
    send(sock_fd, &index, sizeof(index), 0);
    recv(sock_fd, &buffer, sizeof(buffer), 0);

    close(sock_fd);

    /*TODO
        WRITE TO A FILE FROM CHUNK's OFFSET.
    */

    std::ofstream download;
    std::stringstream ss;
    std::string index_str;
    int chunk_size = CHUNK_SIZE;

    ss << index;
    ss >> index_str;
    mtx.lock();
    download.open(file_name + index_str);
    //download.seekp(chunk_size * index, std::ios::beg);
    download << buffer;
    download.close();
    mtx.unlock();

    return 0;
}

int combine_file(std::string file_name, int chunk_number){
    std::ifstream read;
    std::ofstream write(file_name);
    std::string temp_file_name;
    std::string index_string;
    std::stringstream ss;
    char buffer[CHUNK_SIZE];

    for(int i = 0; i < chunk_number; i++){
        ss << i;
        ss >> index_string;
        temp_file_name = file_name + index_string; 
        read.open(temp_file_name);
        read.read(buffer, sizeof(buffer));
        write << buffer;
        std::stringstream().swap(ss);
        read.close();
        remove(temp_file_name.c_str());
    }
    write.close();
    return 0;
}

int register_chunk(std::string file_name, int index){
    int peer_fd;
    std::string file = file_name;
    int chunk_index = index;
    std::string response_message;
    //create socket
    if((peer_fd = socket(AF_INET, SOCK_STREAM , 0)) < 0){
        perror("Failed to create socket"); 
        return -1;
    }

    //connect to server
    if(connect(peer_fd,(struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        perror("Failed to connect"); 
        return -1;
    }

    //send request to the server
    uint32_t request_type = 3;
    uint16_t port = PORT;
    send(peer_fd, &request_type, sizeof(request_type), 0);
    send(peer_fd, &file, sizeof(file), 0);
    send(peer_fd, &chunk_index, sizeof(chunk_index), 0);
    send(peer_fd, &port, sizeof(port), 0);

    if(recv(peer_fd, &response_message, sizeof(response_message), 0) < 0){
        perror("No response from server");
        return -1;
    }
    std::cout<<response_message<<std::endl;
    return 0;
}

int download(std::string file_name){
    std::vector<chunk> chunks;
    std::vector<int> indexes;
    int file_length;

    location_request(file_name, chunks, file_length);
    read_log(file_name, chunks, file_length);

    //pick four chunk to download.
    indexes = select_chunk(chunks);
    
    while(!indexes.empty()){
        for(int i=0; i < indexes.size(); i++){
            //download_file(file_name, indexes[i], chunks[indexes[i]]);
            std::thread download_thread(download_file, file_name, indexes[i], chunks[indexes[i]]);
            download_thread.detach();
            write_log(file_name, chunks);
            chunks[indexes[i]].is_possessed = true;
            register_chunk(file_name, indexes[i]);
        }
        indexes  = select_chunk(chunks);
    }

    for(int i = 0; i < chunks.size(); i++){
        std::cout<<"chunk"<<i<<":"<<std::endl;
        for(int j = 0; j < chunks[i].peers.size(); j++){
            std::cout<<"ip:"<<inet_ntoa(chunks[i].peers[j].address)<<":"<<chunks[i].peers[j].port<<std::endl;
        }
    }

    combine_file(file_name, chunks.size());

    return 0;
}

long get_file_size(std::string filename){
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int parse_command(std::string command){
    std::vector<std::string> parameters;
    std::vector<file_info> file_infos;

    split(command, parameters);
    if(parameters[0] == "show"){
        file_list_request();
        return 0;
    }
    else if(parameters[0] == "share"){
        std::stringstream file_number_string(parameters[1]);
        int file_number = 0;
        struct file_info temp_file_info;
        file_number_string >> file_number;
        for(int i=0; i<file_number; i++){
            temp_file_info.file_name = parameters[i+2];
            temp_file_info.file_length = get_file_size(parameters[i+2]);
            file_infos.push_back(temp_file_info);
        }
        register_request(file_number, file_infos);
    }
    else if(parameters[0] == "download"){
        download(parameters[1]);
    }
    else{
        std::cout<<"command not found"<<std::endl;
    }
    return 0;
}

void user_interface(){
    char buffer[1024];
    while(1){
        std::string command;
        std::cout<<">";
        std::cin.getline(buffer, 1024);
        command = buffer;
        parse_command(command);
    }
    return;
}

void handle_request(int process_fd, std::string file_name, int chunk_index){
    char buffer[CHUNK_SIZE];
    std::ifstream upload_file;
    int chunk_size = CHUNK_SIZE;

    upload_file.open(file_name);
    upload_file.seekg(chunk_size * chunk_index, std::ios::beg);
    upload_file.read(buffer, sizeof(buffer));

    send(process_fd, &buffer, sizeof(buffer), 0);
    upload_file.close();
    close(process_fd);
}

int main(void){
    int process_fd;
    int server_fd;
    struct sockaddr_in client_address, peer_address;
    /* initialize random seed: */
    srand (time(NULL));

    //initailize address structure
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = PF_INET; 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server_address.sin_port = htons(8080); 

    //create thread for user interface
    std::thread user_thread(user_interface);
    user_thread.detach();

    //create socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Failed to create socket"); 
        exit(0);
    }

    //initailize address structure
    bzero(&peer_address, sizeof(peer_address));
    peer_address.sin_family = PF_INET; 
    peer_address.sin_addr.s_addr = INADDR_ANY; 
    peer_address.sin_port = htons(PORT); 

    //bind socket
    if(bind(server_fd, (struct sockaddr*) &peer_address, sizeof(peer_address)) < 0){
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
        else{
            std::string file_name;
            int chunk_index;
            uint32_t request_type;
            recv(process_fd, &file_name, sizeof(file_name), 0);
            recv(process_fd, &chunk_index, sizeof(chunk_index), 0);
            std::cout<<"upload file:" << file_name <<"," <<chunk_index<<std::endl;
            //use multi-thread to handle requests
            std::thread tid(handle_request, process_fd, file_name, chunk_index);
            tid.detach();
        }
    }

    return 0;
}
#ifndef PEERS_HPP
#define PEERS_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <climits>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <vector>
#include <strings.h>

#include <thread>   
#include <mutex>

#define PORT 8082
#define CHUNK_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

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

class client{
/*
int calculate_chunk_number(uint32_t) – Input file length and get how many chunks are needed with this file length.
int register_request(uint16_t, std::vector<file_info>) – Request server to register shared files.
int file_list_request() – Request server to give a list of available files.
int location_request(std::string, std::vector<chunk>&, int &) – Request server to give a list of location where the file is at.
void split(const std::string &, std::vector<string>&, const char) – Split the given sting using a specific delimiter.
file_exist(std::string) – Check if a file exists.
int write_log(std::string, std::vector<chunk>) – Write a log file for recording the status.
int read_log(std::string, std::vector<chunk>, int) – Read a log file to resume the status of previous section.
std::vector<int> select_chunk(std::vector<chunk>) – Choose which chunk to download first.
int select_peer(int) – Choose which peer to download from.
int download_file(std::string, int, struct chunk) – Download a chunk of a file from a peer.
int combine_file(std::string, int ) – Combine all the chunks downloaded.
int register_chunk(std::string, int) – Register possessing a chunk.
int download(std::string) – Download a file.
long get_file_size(std::string) – Get the size of a file.
int parse_command(std::string) – Parse a command entered by users.
int user_interface() – User interface.
void handle_request(int, std::string, int) – Handle download request from other peers.
*/
    private:
        int calculate_chunk_number(uint32_t file_length);
        int register_request(uint16_t number_of_file, std::vector<file_info> file_infos);
        int file_list_request();
        int location_request(std::string file_name, std::vector<chunk>& chunks, int & file_size);
        void split(const std::string& s, std::vector<std::string>& parameters, const char delim);
        bool file_exist(std::string file_name);
        int write_log(std::string file_name, std::vector<chunk> chunks);
        int read_log(std::string file_name, std::vector<chunk>& chunks, int file_length);
        std::vector<int> select_chunk(std::vector<chunk> chunks);
        int download_file(std::string file_name, int index, struct chunk chunk_instance);
        int combine_file(std::string file_name, int chunk_number);
        int register_chunk(std::string file_name, int index);
        int download(std::string file_name);
        long get_file_size(std::string filename);
        int parse_command(std::string command);
        void user_interface();
        void handle_request(int process_fd, std::string file_name, int chunk_index);
    public:
        int execute(void);
};
#endif

#include "server.hpp"

int main(void){
    server server_instance;
    if(server_instance.execute()!= 0){
        perror("server execution fault");
    }
    return 0;
}
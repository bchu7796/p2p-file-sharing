#include "peers.hpp"

int main(void){
    client client_instance;
    if(client_instance.execute()!= 0){
        perror("server execution fault");
    }
    return 0;
}

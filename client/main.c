#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "../commun/commun.h"
#include <asm-generic/socket.h>


int main(){
    int sock = checked(socket(AF_INET, SOCK_STREAM, 0));

    struct sockaddr_in address;   //Adresse du serveur du socket
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(5555);

    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    checked(connect(sock, (struct sockaddr *)&address, sizeof(address)));

    char chemin[1000];


    while (fgets(chemin, sizeof(chemin), stdin) != NULL){
        chemin[strlen(chemin)-1] = '\0';    

        printf("Envoi...\n");
        checked_wr(write(sock, chemin, strlen(chemin)+1));

    }
    close(sock);
    return 0;
}






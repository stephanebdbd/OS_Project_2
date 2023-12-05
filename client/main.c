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

    FILE *file;
    char chemin[1000];
    char *imageBuffer;
    long fileSize;

    while (fgets(chemin, sizeof(chemin), stdin) != NULL){
        chemin[strlen(chemin)-1] = '\0';
        file = fopen(chemin, "rb");
        fileSize = ftell(file);
        imageBuffer = (char *)malloc(fileSize);
        fread(imageBuffer, 1, fileSize, file);
        fclose(file);
        printf("Envoi...\n");
        checked_wr(write(sock, imageBuffer, strlen(imageBuffer)+1));
        free(imageBuffer);
    }
    close(sock);
    return 0;
}




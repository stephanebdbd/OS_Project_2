#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "../commun/commun.h"

void client_socket(int* client_Sock) {
    int sock = checked(socket(AF_INET, SOCK_STREAM, 0));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5555);

    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    client_Sock = checked(connect(sock, (struct sockaddr)&address, sizeof(address)));

    char chemin[1001];
    int taille = 0;

    while (fgets(chemin, sizeof(chemin), stdin) != NULL) {
        chemin[strlen(chemin) - 1] = '\0';
        checked_wr(write(client_Sock, chemin, strlen(chemin) + 1));
        ++taille;

        printf("Envoi...\n");
    }

    close(sock);
}

int main() {
    const int MAX_CLIENTS = 1001;
    pthread_t threads[MAX_CLIENTS];
    int client_sockets[MAX_CLIENTS];
    int nombre_de_client = 0;

    while (nombre_de_client < MAX_CLIENTS) {
        if (pthread_create(&threads[nombre_de_client], NULL, (void)client_socket, (void*)&client_sockets[nombre_de_client]) != 0) {
            perror("Erreur lors de la création du thread");
            close(client_sockets[nombre_de_client]);
        } else {
            nombre_de_client++;
        }
    }

    for (int i = 0; i < nombre_de_client; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < nombre_de_client; ++i) {
        close(client_sockets[i]);
    }

    return 0;
}

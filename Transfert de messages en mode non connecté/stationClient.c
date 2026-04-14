#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    // Création socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Envoi "Bonjour"
    strcpy(buffer, "Bonjour");
    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&server_addr, addr_len);

    printf("Message envoyé: Bonjour\n");

    // Réception des messages
    while (1) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&server_addr, &addr_len);

        buffer[n] = '\0';
        printf("Reçu: %s", buffer);

        if (strstr(buffer, "Au revoir") != NULL)
            break;
    }

    close(sockfd);
    return 0;
}

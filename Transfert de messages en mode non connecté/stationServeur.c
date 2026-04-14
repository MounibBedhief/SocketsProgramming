#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Création socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Serveur UDP en attente...\n");

    // Réception du message "Bonjour"
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
             (struct sockaddr *)&client_addr, &addr_len);

    printf("Client: %s\n", buffer);

    // Envoi de l'heure 60 fois
    for (int i = 0; i < 60; i++) {
        time_t now = time(NULL);
        snprintf(buffer, BUFFER_SIZE, "Il est %s", ctime(&now));

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr *)&client_addr, addr_len);

        sleep(1); // délai 1 seconde
    }

    // Envoi "Au revoir"
    strcpy(buffer, "Au revoir");
    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&client_addr, addr_len);

    close(sockfd);
    return 0;
}

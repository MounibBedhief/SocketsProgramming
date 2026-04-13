#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Adresse serveur (localhost)
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    printf("Connecting to server 127.0.0.1:%d...\n", PORT);

    // Connexion TCP
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Connected to server. Sending messages...\n");

    for(int i = 0; i < 60; i++) {
        time_t now = time(NULL);
        snprintf(buffer, sizeof(buffer), "%s", ctime(&now));

        printf("Sending message %d: %s", i + 1, buffer);
        send(sock, buffer, strlen(buffer), 0);

        sleep(1); // 1 seconde entre chaque envoi
    }

    printf("All messages sent. Closing connection.\n");

    close(sock);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 80
#define SERVER "example.com"

int main() {
    int sock;
    struct sockaddr_in server_addr;
    struct hostent *host;
    char request[2048];
    char response[4096];
    int bytes;

    // 1. Création socket TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    // 2. Résolution DNS
    host = gethostbyname(SERVER);
    if (host == NULL) {
        perror("DNS error");
        return 1;
    }

    // 3. Configuration serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    // 4. Connexion
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to %s\n", SERVER);

    // 5. Lecture requête HTTP depuis clavier
    printf("Enter HTTP request (finish with empty line):\n");

    memset(request, 0, sizeof(request));

    while (1) {
        char line[512];
        fgets(line, sizeof(line), stdin);

        // fin de requête (ligne vide)
        if (strcmp(line, "\n") == 0) {
            strcat(request, "\r\n");
            break;
        }

        strcat(request, line);
    }

    // 6. Envoi requête
    send(sock, request, strlen(request), 0);

    // 7. Lecture réponse serveur
    printf("\n--- SERVER RESPONSE ---\n");

    while ((bytes = recv(sock, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes] = '\0';
        printf("%s", response);
    }

    // 8. Fermeture socket
    close(sock);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define PORT 8080

void get_timestamp(char *buffer, size_t size) {
    struct timeval tv;
    struct tm *timeinfo;
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);
    
    strftime(buffer, size, "%H:%M:%S", timeinfo);
    char temp[size];
    snprintf(temp, size, "%s.%06ld", buffer, tv.tv_usec);
    strcpy(buffer, temp);
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[4096];
    char timestamp[64];
    pid_t client_id = getpid();
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client ECHO #%d: Connexion au serveur...\n", timestamp, client_id);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de connexion");
        return 1;
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client ECHO #%d: Connecté\n", timestamp, client_id);
    
    // Lire le menu du serveur
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer) - 1, 0);
    printf("%s", buffer);
    
    // Envoyer le choix du service: ECHO (service 1)
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client ECHO #%d: Envoi du choix de service (1)\n", timestamp, client_id);
    send(sock, "1\n", 2, 0);
    sleep(1);
    
    // Lire la confirmation du service
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer) - 1, 0);
    printf("%s", buffer);
    
    // Envoyer 10 messages
    for(int i = 0; i < 10; i++) {
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "Message ECHO %d du client %d\n", i + 1, client_id);
        
        get_timestamp(timestamp, sizeof(timestamp));
        printf("[%s] Client ECHO #%d: Envoi du message %d\n", timestamp, client_id, i + 1);
        
        send(sock, buffer, strlen(buffer), 0);
        
        // Lire l'écho
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer) - 1, 0);
        printf("[%s] Client ECHO #%d: Reçu - %s", timestamp, client_id, buffer);
        
        sleep(1);
    }
    
    // Lire la fin du service
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer) - 1, 0);
    printf("%s", buffer);
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client ECHO #%d: Déconnexion\n", timestamp, client_id);
    
    close(sock);
    return 0;
}
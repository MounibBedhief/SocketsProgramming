#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

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
    char buffer[1024];
    char timestamp[64];
    pid_t client_id = getpid();
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d: Connexion au serveur 127.0.0.1:%d...\n", timestamp, client_id, PORT);
    
    // Connexion TCP
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de connexion");
        return 1;
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d: Connecté. Envoi des messages...\n", timestamp, client_id);
    
    // Envoi de 60 messages
    for(int i = 0; i < 60; i++) {
        time_t now = time(NULL);
        snprintf(buffer, sizeof(buffer), "[Message %d] %s", i + 1, ctime(&now));
        
        get_timestamp(timestamp, sizeof(timestamp));
        printf("[%s] Client #%d: Envoi du message %d\n", timestamp, client_id, i + 1);
        
        send(sock, buffer, strlen(buffer), 0);
        sleep(1);
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d: Tous les messages envoyés. Fermeture.\n", timestamp, client_id);
    
    close(sock);
    return 0;
}
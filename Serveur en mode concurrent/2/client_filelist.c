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
    printf("[%s] Client FILELIST #%d: Connexion au serveur...\n", timestamp, client_id);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur de connexion");
        return 1;
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client FILELIST #%d: Connecté\n", timestamp, client_id);
    
    // Lire le menu du serveur
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer) - 1, 0);
    printf("%s", buffer);
    
    // Envoyer le choix du service: FILELIST (service 3)
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client FILELIST #%d: Envoi du choix de service (3)\n", timestamp, client_id);
    send(sock, "3\n", 2, 0);
    
    sleep(1);
    
    // Lire tous les résultats
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client FILELIST #%d: Réception de la liste des fichiers...\n", timestamp, client_id);
    
    for(int i = 0; i < 30; i++) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) break;
        printf("%s", buffer);
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client FILELIST #%d: Déconnexion\n", timestamp, client_id);
    
    close(sock);
    return 0;
}
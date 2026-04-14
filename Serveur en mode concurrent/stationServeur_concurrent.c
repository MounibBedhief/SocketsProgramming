#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>

#define PORT 8080
#define MAX_CLIENTS 100

int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure pour passer les données au thread
typedef struct {
    int socket;
    int client_id;
} client_args_t;

// Fonction pour obtenir l'heure actuelle en microsecondes
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

// Fonction exécutée par chaque thread client
void* handle_client(void* arg) {
    client_args_t *args = (client_args_t *)arg;
    int new_socket = args->socket;
    int client_id = args->client_id;
    free(args);
    
    char buffer[1024] = {0};
    char timestamp[64];
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d connecté\n", timestamp, client_id);
    
    // Réception des messages du client
    for(int i = 0; i < 60; i++) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(new_socket, buffer, 1024);
        
        if (bytes_read <= 0) {
            break;
        }
        
        get_timestamp(timestamp, sizeof(timestamp));
        printf("[%s] Client #%d - Message %d: %s", timestamp, client_id, i+1, buffer);
    }
    
    close(new_socket);
    
    // Mise à jour du compteur de clients (thread-safe)
    pthread_mutex_lock(&client_mutex);
    client_count--;
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d déconnecté (Clients actifs: %d)\n", timestamp, client_id, client_count);
    pthread_mutex_unlock(&client_mutex);
    
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_id_counter = 0;
    
    // Création socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Permettre la réutilisation du port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    // Listen
    listen(server_fd, MAX_CLIENTS);
    
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Serveur en attente sur le port %d...\n", timestamp, PORT);
    
    // Boucle d'acceptation des clients
    while(1) {
        struct sockaddr_in client_addr;
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
        
        if (new_socket < 0) {
            perror("accept");
            continue;
        }
        
        // Mise à jour du compteur (thread-safe)
        pthread_mutex_lock(&client_mutex);
        client_count++;
        client_id_counter++;
        int current_client_id = client_id_counter;
        pthread_mutex_unlock(&client_mutex);
        
        // Création d'un thread pour gérer ce client
        pthread_t thread_id;
        client_args_t *args = (client_args_t *)malloc(sizeof(client_args_t));
        args->socket = new_socket;
        args->client_id = current_client_id;
        
        if (pthread_create(&thread_id, NULL, handle_client, args) != 0) {
            perror("pthread_create");
            free(args);
            close(new_socket);
        } else {
            pthread_detach(thread_id);
        }
    }
    
    close(server_fd);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096

// Service codes
#define SERVICE_ECHO 1
#define SERVICE_SYSINFO 2
#define SERVICE_FILELIST 3

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

// SERVICE 1: Echo - Retourne les messages reçus
void service_echo(int socket, int client_id) {
    char buffer[BUFFER_SIZE] = {0};
    char timestamp[64];
    
    send(socket, "=== SERVICE ECHO ===\n", 21, 0);
    send(socket, "Envoyez 10 messages (ils seront retournés)\n", 43, 0);
    
    for(int i = 0; i < 10; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(socket, buffer, BUFFER_SIZE - 1);
        
        if (bytes_read <= 0) break;
        
        get_timestamp(timestamp, sizeof(timestamp));
        printf("[%s] Client #%d [ECHO] Reçu: %s", timestamp, client_id, buffer);
        
        // Echo back to client
        send(socket, "Echo: ", 6, 0);
        send(socket, buffer, bytes_read, 0);
    }
    
    send(socket, "=== FIN SERVICE ECHO ===\n", 25, 0);
}

// SERVICE 2: System Info - Retourne des infos système
void service_sysinfo(int socket, int client_id) {
    char buffer[BUFFER_SIZE] = {0};
    char timestamp[64];
    
    send(socket, "=== SERVICE SYSINFO ===\n", 25, 0);
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d [SYSINFO] Requête reçue\n", timestamp, client_id);
    
    // Nombre de processus
    FILE *fp = popen("ps aux | wc -l", "r");
    if (fp != NULL) {
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            send(socket, "Nombre de processus: ", 21, 0);
            send(socket, buffer, strlen(buffer), 0);
        }
        pclose(fp);
    }
    
    // Uptime
    memset(buffer, 0, BUFFER_SIZE);
    fp = popen("uptime", "r");
    if (fp != NULL) {
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            send(socket, "Uptime: ", 8, 0);
            send(socket, buffer, strlen(buffer), 0);
        }
        pclose(fp);
    }
    
    // Load average
    memset(buffer, 0, BUFFER_SIZE);
    fp = popen("cat /proc/loadavg | cut -d' ' -f1-3", "r");
    if (fp != NULL) {
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            send(socket, "Load Average: ", 14, 0);
            send(socket, buffer, strlen(buffer), 0);
        }
        pclose(fp);
    }
    
    send(socket, "=== FIN SERVICE SYSINFO ===\n", 28, 0);
}

// SERVICE 3: File List - Liste les fichiers du répertoire courant
void service_filelist(int socket, int client_id) {
    char buffer[BUFFER_SIZE] = {0};
    char timestamp[64];
    
    send(socket, "=== SERVICE FILELIST ===\n", 26, 0);
    send(socket, "Fichiers du répertoire courant:\n", 32, 0);
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d [FILELIST] Requête reçue\n", timestamp, client_id);
    
    DIR *dir = opendir(".");
    if (dir != NULL) {
        struct dirent *entry;
        int count = 0;
        while ((entry = readdir(dir)) != NULL && count < 20) {
            if (entry->d_name[0] != '.') {  // Skip hidden files
                snprintf(buffer, BUFFER_SIZE, "  - %s\n", entry->d_name);
                send(socket, buffer, strlen(buffer), 0);
                count++;
            }
        }
        closedir(dir);
    }
    
    send(socket, "=== FIN SERVICE FILELIST ===\n", 29, 0);
}

// Fonction exécutée par chaque thread client
void* handle_client(void* arg) {
    client_args_t *args = (client_args_t *)arg;
    int socket = args->socket;
    int client_id = args->client_id;
    free(args);
    
    char buffer[BUFFER_SIZE] = {0};
    char timestamp[64];
    int service;
    
    pthread_mutex_lock(&client_mutex);
    client_count++;
    pthread_mutex_unlock(&client_mutex);
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] Client #%d connecté\n", timestamp, client_id);
    
    // Afficher le menu des services
    const char *menu = "\n=== SERVEUR MULTI-SERVICE ===\n"
                       "Choisissez un service:\n"
                       "1 - SERVICE ECHO (echo messages)\n"
                       "2 - SERVICE SYSINFO (info système)\n"
                       "3 - SERVICE FILELIST (liste fichiers)\n"
                       "Entrez le numéro du service (1, 2 ou 3): ";
    send(socket, menu, strlen(menu), 0);
    
    // Lire le choix du client
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(socket, buffer, BUFFER_SIZE - 1);
    
    if (bytes_read > 0) {
        service = atoi(buffer);
        
        get_timestamp(timestamp, sizeof(timestamp));
        printf("[%s] Client #%d a choisi le service %d\n", timestamp, client_id, service);
        
        // Exécuter le service demandé
        switch(service) {
            case SERVICE_ECHO:
                service_echo(socket, client_id);
                break;
            case SERVICE_SYSINFO:
                service_sysinfo(socket, client_id);
                break;
            case SERVICE_FILELIST:
                service_filelist(socket, client_id);
                break;
            default:
                send(socket, "Service invalide!\n", 19, 0);
                break;
        }
    }
    
    close(socket);
    
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
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }
    
    // Listen
    listen(server_fd, MAX_CLIENTS);
    
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    printf("[%s] === SERVEUR MULTI-SERVICE DÉMARRÉ ===\n", timestamp);
    printf("[%s] En attente sur le port %d...\n", timestamp, PORT);
    printf("[%s] Services disponibles:\n", timestamp);
    printf("[%s]   - SERVICE 1: ECHO\n", timestamp);
    printf("[%s]   - SERVICE 2: SYSINFO\n", timestamp);
    printf("[%s]   - SERVICE 3: FILELIST\n", timestamp);
    printf("[%s] \n", timestamp);
    
    // Boucle d'acceptation des clients
    while(1) {
        struct sockaddr_in client_addr;
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
        
        if (new_socket < 0) {
            perror("accept");
            continue;
        }
        
        // Mise à jour du compteur
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
================================================================================
SERVEUR MULTI-SERVICE CONCURRENT - SOCKET PROGRAMMING
================================================================================

DESCRIPTION
================================================================================

Ce projet implémente un serveur TCP concurrent mono-protocole multi-services 
capable de gérer plusieurs clients simultanément, chacun accédant à des services 
distincts.

Le serveur utilise des threads POSIX pour traiter les requêtes clientes en 
parallèle, permettant ainsi une véritable concurrence au niveau des services.


ARCHITECTURE
================================================================================

MODÈLE CLIENT/SERVEUR:

    ┌─────────────┐         TCP Socket          ┌──────────────┐
    │  Client 1   │◄───────────────────────────►│              │
    ├─────────────┤                             │              │
    │  Client 2   │◄─────┐      SERVEUR         │  Thread 1    │
    ├─────────────┤      │   Multi-Service      │  Thread 2    │
    │  Client 3   │◄─────┴────────────────────► │  Thread 3    │
    └─────────────┘                             └──────────────┘

FONCTIONNEMENT:

1. Serveur: Écoute les connexions entrantes sur le port 8080
2. Acceptation: Crée un nouveau thread pour chaque client
3. Services: Le client choisit un service (ECHO, SYSINFO, ou FILELIST)
4. Traitement: Le thread exécute le service demandé en parallèle
5. Déconnexion: Le thread se termine après le traitement


SERVICES DISPONIBLES
================================================================================

SERVICE 1: ECHO (Echo Messages)
--------------------------------------------------------------------------------

Description: Reçoit des messages du client et les retourne (echo)

Protocole:
  Client → Serveur: "1"              (Sélection du service)
  Client → Serveur: Message 1        (Envoi message)
  Serveur → Client: "Echo: Message 1" (Réponse)
  [Répète 10 fois]

Cas d'usage: Test de communication bidirectionnelle


SERVICE 2: SYSINFO (System Information)
--------------------------------------------------------------------------------

Description: Retourne des informations système du serveur

Données retournées:
  - Nombre de processus actifs (via ps aux | wc -l)
  - Uptime du serveur (durée depuis le dernier redémarrage)
  - Load Average (charge système 1m, 5m, 15m)

Protocole:
  Client → Serveur: "2"                  (Sélection du service)
  Serveur → Client: Données système      (Réponse unique)

SERVICE 3: FILELIST (File Listing)
--------------------------------------------------------------------------------

Description: Liste les fichiers du répertoire courant du serveur

Données retournées:
  - Liste des fichiers du répertoire courant (max 20)
  - Exclut les fichiers cachés (commençant par .)

Protocole:
  Client → Serveur: "3"              (Sélection du service)
  Serveur → Client: Liste de fichiers (Réponse)


COMPILATION ET EXÉCUTION
================================================================================

COMPILATION MANUELLE
--------------------------------------------------------------------------------

Compiler le serveur (avec support pthread):
  gcc -pthread -o stationServeur_multiservice stationServeur_multiservice.c

Compiler les clients:
  gcc -o client_echo client_echo.c
  gcc -o client_sysinfo client_sysinfo.c
  gcc -o client_filelist client_filelist.c


EXÉCUTION AVEC LE SCRIPT DE TEST
--------------------------------------------------------------------------------

  chmod +x test_multiservice.sh
  ./test_multiservice.sh

Le script automatise:
  ✓ Compilation de tous les programmes
  ✓ Lancement du serveur
  ✓ Exécution de 3 scénarios de test
  ✓ Collecte des logs
  ✓ Arrêt du serveur


SCÉNARIOS DE TEST
================================================================================

SCÉNARIO 1: Deux clients ECHO en parallèle
--------------------------------------------------------------------------------

Commande:
  ./client_echo &  # Client 1
  ./client_echo &  # Client 2

Résultat attendu: Les deux clients s'exécutent simultanément (~12 secondes 
au total)


SCÉNARIO 2: Deux clients SYSINFO en parallèle
--------------------------------------------------------------------------------

Commande:
  ./client_sysinfo &  # Client 1
  ./client_sysinfo &  # Client 2

Résultat attendu: Traitement très rapide (~2 secondes au total)


SCÉNARIO 3: Services mixtes en parallèle
--------------------------------------------------------------------------------

Commande:
  ./client_echo &       # Service ECHO
  ./client_sysinfo &    # Service SYSINFO
  ./client_filelist &   # Service FILELIST

Résultat attendu: Les 3 services s'exécutent en parallèle (~11 secondes au 
total)



CONFIGURATION
================================================================================

PORT PAR DÉFAUT:
  #define PORT 8080

LIMITE DE CLIENTS SIMULTANÉS:
  #define MAX_CLIENTS 100

TAILLE DES BUFFERS:
  #define BUFFER_SIZE 4096

Modifiez ces valeurs dans stationServeur_multiservice.c si nécessaire.


CONCEPTS CLÉS
================================================================================

THREADS POSIX (pthreads):
  - pthread_create(): Crée un nouveau thread
  - pthread_detach(): Permet au thread de se libérer automatiquement
  - pthread_mutex_lock/unlock(): Protection des sections critiques

SOCKETS TCP:
  - socket(): Crée un socket
  - bind(): Attache le socket à un port
  - listen(): Écoute les connexions
  - accept(): Accepte une nouvelle connexion
  - send/recv(): Communication

CONCURRENCE:
  ✓ Un thread par client = véritable parallélisme
  ✓ Services indépendants = pas de blocage croisé
  ✓ Mutex = accès cohérent aux données partagées


FLUX DE COMMUNICATION DÉTAILLÉ
================================================================================

ÉTAPE 1: DÉMARRAGE DU SERVEUR
  1. Créer un socket TCP
  2. Binder le socket au port 8080
  3. Écouter les connexions entrantes
  4. Afficher les services disponibles

ÉTAPE 2: CONNEXION D'UN CLIENT
  1. Client crée un socket
  2. Client se connecte au serveur (127.0.0.1:8080)
  3. Serveur accepte la connexion et crée un thread
  4. Serveur envoie le menu des services

ÉTAPE 3: SÉLECTION DU SERVICE
  1. Client lit le menu et choisit un service (1, 2 ou 3)
  2. Client envoie son choix au serveur
  3. Serveur reçoit le choix et lance le service approprié

ÉTAPE 4: EXÉCUTION DU SERVICE
  
  Si SERVICE ECHO (1):
    - Serveur demande 10 messages
    - Pour chaque message reçu:
      * Serveur affiche "Reçu: [message]"
      * Serveur envoie "Echo: [message]" au client
    - Après 10 messages, serveur envoie "FIN SERVICE ECHO"
  
  Si SERVICE SYSINFO (2):
    - Serveur exécute les commandes système
    - Serveur compile les résultats
    - Serveur envoie tous les résultats d'un coup
  
  Si SERVICE FILELIST (3):
    - Serveur ouvre le répertoire courant
    - Serveur liste les fichiers
    - Serveur envoie la liste

ÉTAPE 5: DÉCONNEXION
  1. Client ferme la connexion
  2. Serveur ferme le socket
  3. Thread se termine
  4. Serveur retourne à l'écoute


INFORMATIONS TECHNIQUES
================================================================================

SYSTÈME D'EXPLOITATION:
  Linux (testé sur Arch)

LANGAGE:
  C (Standard C99 ou supérieur)

BIBLIOTHÈQUES REQUISES:
  - pthread: Threads POSIX
  - unistd.h: API POSIX standard
  - sys/socket.h: Programmation socket
  - arpa/inet.h: Fonctions réseau

COMPILATION:
  gcc -pthread [fichier.c] -o [executable]

EXÉCUTION:
  ./[executable]


FICHIERS DE LOGS
================================================================================

Le script test_multiservice.sh crée les fichiers de logs suivants:

  test_logs_multiservice/server.log       # Logs du serveur
  test_logs_multiservice/echo1.log        # Client ECHO #1
  test_logs_multiservice/echo2.log        # Client ECHO #2
  test_logs_multiservice/sysinfo1.log     # Client SYSINFO #1
  test_logs_multiservice/sysinfo2.log     # Client SYSINFO #2
  test_logs_multiservice/filelist_mixed.log  # Client FILELIST
  test_logs_multiservice/echo_mixed.log   # Client ECHO (scénario mixte)
  test_logs_multiservice/sysinfo_mixed.log   # Client SYSINFO (scénario mixte)




Bonne programmation!


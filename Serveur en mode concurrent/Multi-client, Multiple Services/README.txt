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

    ┌─────────────┐         TCP Socket         ┌──────────────┐
    │  Client 1   │◄───────────────────────────►│              │
    ├─────────────┤                             │              │
    │  Client 2   │◄─────┐      SERVEUR        │  Thread 1    │
    ├─────────────┤      │   Multi-Service     │  Thread 2    │
    │  Client 3   │◄─────┴────────────────────►│  Thread 3    │
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

Exemple de sortie:
  === SERVICE ECHO ===
  Envoyez 10 messages (ils seront retournés)
  [11:58:22] Client #17106: Envoi du message 1
  [11:58:22] Client #17106: Reçu - Echo: Message ECHO 1 du client 17106


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

Exemple de sortie:
  === SERVICE SYSINFO ===
  Nombre de processus: 279
  Uptime:  11:58:34 up  1:55,  1 user,  load average: 0.60, 0.83, 0.94
  Load Average: 0.60 0.83 0.94
  === FIN SERVICE SYSINFO ===


SERVICE 3: FILELIST (File Listing)
--------------------------------------------------------------------------------

Description: Liste les fichiers du répertoire courant du serveur

Données retournées:
  - Liste des fichiers du répertoire courant (max 20)
  - Exclut les fichiers cachés (commençant par .)

Protocole:
  Client → Serveur: "3"              (Sélection du service)
  Serveur → Client: Liste de fichiers (Réponse)

Exemple de sortie:
  === SERVICE FILELIST ===
  Fichiers du répertoire courant:
    - stationServeur_multiservice
    - client_echo
    - client_sysinfo
    - client_filelist
    - test_multiservice.sh
  === FIN SERVICE FILELIST ===


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


RÉSULTATS DE TEST
================================================================================

Scénario                  | Clients                              | Temps Total | Résultat
========================================================================================
ECHO (2 clients)          | 2 × ECHO                             | ~12s        | Parallèle
SYSINFO (2 clients)       | 2 × SYSINFO                          | ~2s         | Rapide
Services mixtes           | 1 ECHO + 1 SYSINFO + 1 FILELIST      | ~11s        | Tous parallèles


OBSERVATION CLÉ: PARALLÉLISME CONFIRMÉ
================================================================================

Temps ECHO (10 messages × 1s):      ~10s
Temps SYSINFO (requête simple):     ~1s
Temps FILELIST (lecture répertoire): ~1s

Mode SÉQUENTIEL attendu: 10 + 1 + 1 = 12s
Mode PARALLÈLE observé:  ~11s        ✓ CONFIRMÉ!

Cela démontre que les services s'exécutent bien en parallèle sur des threads 
différents, et que le serveur traite plusieurs clients simultanément.


SYNCHRONISATION THREAD-SAFE
================================================================================

Le serveur utilise un mutex POSIX pour protéger les données partagées:

  pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
  int client_count = 0;  // Compteur protégé

  pthread_mutex_lock(&client_mutex);
  client_count++;        // Opération atomique
  pthread_mutex_unlock(&client_mutex);

Protège:
  ✓ Compteur de clients connectés
  ✓ ID unique pour chaque client
  ✓ Cohérence des données partagées


STRUCTURE DES FICHIERS
================================================================================

Transfert de messages en mode connecté/
  ├── stationServeur_multiservice.c   # Serveur concurrent multi-services
  ├── client_echo.c                   # Client pour service ECHO
  ├── client_sysinfo.c                # Client pour service SYSINFO
  ├── client_filelist.c               # Client pour service FILELIST
  ├── test_multiservice.sh            # Script de test automatisé
  └── README.txt                      # Cette documentation


CONFIGURATION
================================================================================

PORT PAR DÉFAUT:
  #define PORT 8080

LIMITE DE CLIENTS SIMULTANÉS:
  #define MAX_CLIENTS 100

TAILLE DES BUFFERS:
  #define BUFFER_SIZE 4096

Modifiez ces valeurs dans stationServeur_multiservice.c si nécessaire.


DÉPANNAGE
================================================================================

PORT 8080 DÉJÀ UTILISÉ
--------------------------------------------------------------------------------

Vérifier quel processus utilise le port:
  lsof -i :8080

Terminer le processus:
  kill -9 <PID>

Ou directement:
  sudo fuser -k 8080/tcp


CLIENTS NE SE CONNECTENT PAS
--------------------------------------------------------------------------------

Vérifier que le serveur est actif:
  ps aux | grep stationServeur_multiservice

Vérifier les logs du serveur:
  cat test_logs_multiservice/server.log


PERMISSIONS INSUFFISANTES POUR LES COMMANDES SYSTÈME
--------------------------------------------------------------------------------

Le service SYSINFO utilise ps, uptime, et cat /proc/loadavg. Si ces commandes 
ne sont pas disponibles, modifiez la fonction service_sysinfo() dans le serveur.


AMÉLIORATIONS POSSIBLES
================================================================================

1. GESTION D'ERREURS AMÉLIORÉE:
   - Vérification de tous les appels systèmes
   - Fermeture propre des ressources

2. NOUVEAUX SERVICES:
   - Service de transfert de fichiers
   - Service de calcul distant
   - Service de chat multi-utilisateur

3. AUTHENTIFICATION:
   - Ajout d'un système de login
   - Contrôle d'accès par service

4. PERSISTANCE:
   - Base de données pour historique
   - Logs centralisés

5. PERFORMANCE:
   - Pool de threads (au lieu de thread par client)
   - Utilisation de epoll ou select pour meilleure scalabilité


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


EXEMPLE D'EXÉCUTION COMPLÈTE
================================================================================

Terminal 1 - Démarrer le serveur:

  $ ./stationServeur_multiservice
  [11:58:19.836166] === SERVEUR MULTI-SERVICE DÉMARRÉ ===
  [11:58:19.836166] En attente sur le port 8080...
  [11:58:19.836166] Services disponibles:
  [11:58:19.836166]   - SERVICE 1: ECHO
  [11:58:19.836166]   - SERVICE 2: SYSINFO
  [11:58:19.836166]   - SERVICE 3: FILELIST


Terminal 2 - Lancer un client ECHO:

  $ ./client_echo
  [11:58:21.843529] Client ECHO #17106: Connexion au serveur...
  [11:58:21.843727] Client ECHO #17106: Connecté
  
  === SERVEUR MULTI-SERVICE ===
  Choisissez un service:
  1 - SERVICE ECHO (echo messages)
  2 - SERVICE SYSINFO (info système)
  3 - SERVICE FILELIST (liste fichiers)
  Entrez le numéro du service (1, 2 ou 3): [11:58:21.844144] Client ECHO #17106: Envoi du choix de service (1)
  === SERVICE ECHO ===
  Envoyez 10 messages (ils seront retournés)
  [11:58:22.844334] Client ECHO #17106: Envoi du message 1
  [11:58:22.844334] Client ECHO #17106: Reçu - Echo: Message ECHO 1 du client 17106
  ...
  [11:58:32.847016] Client ECHO #17106: Déconnexion


Terminal 1 - Logs du serveur (correspondants):

  [11:58:21.844144] Client #17106 connecté
  [11:58:21.844144] Client #17106 a choisi le service 1
  [11:58:22.844334] Client #17106 [ECHO] Reçu: Message ECHO 1 du client 17106
  ...
  [11:58:32.847016] Client #17106 déconnecté (Clients actifs: 0)


INFORMATIONS TECHNIQUES
================================================================================

SYSTÈME D'EXPLOITATION:
  Linux (testé sur Ubuntu/Fedora)

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


PERFORMANCE ET SCALABILITÉ
================================================================================

POINTS FORTS:
  ✓ Vrai parallélisme avec threads
  ✓ Réponse rapide (pas de file d'attente)
  ✓ Services indépendants
  ✓ Thread-safe avec mutex

LIMITATIONS:
  - Maximum ~100 clients simultanés (configurable)
  - Chaque client = 1 thread = consommation mémoire
  - Pas optimisé pour 10000+ connexions simultanées

POUR UNE MEILLEURE SCALABILITÉ:
  - Utiliser un pool de threads
  - Remplacer threads par epoll/kqueue
  - Implémenter asynchrone I/O


AUTEUR ET ATTRIBUTION
================================================================================

Projet: Serveur Multi-Service Concurrent
Langage: C (POSIX)
Type: Programmation socket - Communication réseau
Date: Avril 2026
Auteur: Mounib Bedhief


CONCEPTS APPRIS
================================================================================

✓ Programmation socket (TCP/IP)
✓ Threads concurrents (pthreads)
✓ Synchronisation (mutex)
✓ Architecture client/serveur
✓ Services multiples sur un même port
✓ Exécution parallèle vs séquentielle
✓ Gestion de ressources
✓ Communication réseau


COMMANDES UTILES
================================================================================

Vérifier les services en écoute:
  netstat -tuln | grep 8080
  ss -tuln | grep 8080
  lsof -i :8080

Voir les threads actifs:
  ps -eLf | grep [processus]

Mesurer la charge système:
  top
  htop
  uptime

Capturer le trafic réseau:
  tcpdump -i lo port 8080

Voir les logs en temps réel:
  tail -f test_logs_multiservice/server.log


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


PROTOCOLE DE COMMUNICATION
================================================================================

FORMAT DES MESSAGES:

Client → Serveur (Service Selection):
  Format: "[0-9]\n"
  Exemple: "1\n" (sélectionner ECHO)
  Exemple: "2\n" (sélectionner SYSINFO)
  Exemple: "3\n" (sélectionner FILELIST)

Serveur → Client (Menu):
  Format: String terminé par newline
  Contient les 3 options de service

Serveur → Client (Réponse):
  Format: Dépend du service
  ECHO: "Echo: " + message du client
  SYSINFO: Données système compilées
  FILELIST: Liste des fichiers

Client → Serveur (Messages ECHO):
  Format: Texte libre + newline
  Exemple: "Message ECHO 1 du client 17106\n"


PRÉREQUIS SYSTÈME
================================================================================

MINIMUM:
  - 512 MB RAM
  - Linux kernel 2.6+
  - GCC 4.8+
  - POSIX compatible

RECOMMANDÉ:
  - 2GB+ RAM
  - Linux kernel 4.0+
  - GCC 7.0+
  - Bash 4.0+


RÉFÉRENCES ET DOCUMENTATION
================================================================================

POSIX Threads:
  man 7 pthreads
  man 3 pthread_create
  man 3 pthread_mutex_init

Socket Programming:
  man 2 socket
  man 2 bind
  man 2 listen
  man 2 accept
  man 2 send
  man 2 recv

Linux System:
  man ps
  man uptime
  man proc

Standards:
  POSIX.1-2008
  RFC 793 (TCP)
  RFC 791 (IP)


NOTES IMPORTANTES
================================================================================

1. Ce projet est à titre éducatif
2. Non recommandé pour usage en production sans modifications de sécurité
3. Ajouter la gestion d'erreurs pour code robuste
4. Tester avec des outils comme telnet ou nc avant utilisation réelle
5. Documenter toutes les modifications apportées


FIN DE LA DOCUMENTATION
================================================================================

Pour toute question ou amélioration suggérée, consultez le code source
directement.

Bonne programmation!

================================================================================
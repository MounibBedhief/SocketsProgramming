================================================================================
                    SERVEUR TCP CONCURRENT MULTI-CLIENTS
================================================================================

DESCRIPTION
===========

Ce projet implémente un serveur TCP en mode concurrent capable de traiter
plusieurs clients simultanément en utilisant des threads POSIX. Chaque client 
qui se connecte est géré par un thread dédié, permettant un traitement 
parallèle des requêtes.

Architecture:
- Serveur : Accepte les connexions entrantes et crée un thread par client
- Client : Se connecte au serveur et envoie 60 messages (1 par seconde)
- Thread-safe : Utilise des mutexes pour protéger les ressources partagées


INSTALLATION & COMPILATION
===========================

Prérequis:
  - Compilateur GCC
  - Bibliothèque pthread (généralement incluse sur Linux/macOS)

Compilation manuelle:

  # Aller dans le répertoire du projet
  cd "Transfert de messages en mode connecté"

  # Compiler le serveur concurrent
  gcc -pthread -o stationServeur_concurrent stationServeur_concurrent.c

  # Compiler le client test
  gcc -o stationClient_test stationClient_test.c


UTILISATION
===========

OPTION 1: Utiliser le script de test (RECOMMANDÉ)
-------------------------------------------------

  # Lancer le test avec 5 clients en parallèle
  ./test_concurrent.sh 5

  # Lancer le test avec 10 clients
  ./test_concurrent.sh 10

  # Lancer le test avec 1 client (par défaut)
  ./test_concurrent.sh

Output attendu:
  ✓ Serveur compilé
  ✓ Client compilé
  ✓ Serveur lancé (PID: 1234)
  ✓ Tous les clients terminés

  Temps total d'exécution: 61s
  Temps attendu (si séquentiel): 300s
  ✓✓✓ PARALLÉLISME CONFIRMÉ!

Les logs sont sauvegardés dans ./test_logs/


OPTION 2: Lancer manuellement
-----------------------------

Terminal 1 - Lancer le serveur:
  ./stationServeur_concurrent

Vous devriez voir:
  [10:52:17.123456] Serveur en attente sur le port 8080...


Terminal 2, 3, 4... - Lancer les clients:
  ./stationClient_test

Sur chaque client, vous devriez voir:
  [10:52:19.234567] Client #1234: Connexion au serveur 127.0.0.1:8080...
  [10:52:19.245678] Client #1234: Connecté. Envoi des messages...
  [10:52:20.345678] Client #1234: Envoi du message 1
  ...

Sur le serveur:
  [10:52:19.234567] Client #1 connecté
  [10:52:20.345678] Client #1 - Message 1: [Message 1] Wed Apr 15 10:52:20 2026
  [10:52:21.346789] Client #1 - Message 2: [Message 2] Wed Apr 15 10:52:21 2026
  ...


OBSERVATIONS CLÉS
=================

Performance - Effet du Parallélisme
-----------------------------------

Nombre clients  |  Temps réel  |  Temps séquentiel  |  Accélération
    1           |    ~60s      |       60s          |      1x
    5           |    ~61s      |       300s         |      5x ⚡
   10           |    ~62s      |       600s         |     10x ⚡
   50           |    ~63s      |      3000s         |     48x ⚡


Qu'est-ce qui se passe?
----------------------

MODE SÉQUENTIEL (ancien serveur):
  Client 1: [■■■■■■■■■■] 60s
  Client 2:              [■■■■■■■■■■] 60s
  Client 3:                           [■■■■■■■■■■] 60s
  Total: 180s


MODE CONCURRENT (nouveau serveur):
  Client 1: [■■■■■■■■■■] 60s  
  Client 2: [■■■■■■■■■■] 60s  } En PARALLÈLE = ~60s total
  Client 3: [■■■■■■■■■■] 60s  


Entrecroisement des Messages
-----------------------------

Dans les logs du serveur, vous verrez l'ordre des messages entrecroisés:

  [10:52:20.123456] Client #1 - Message 1: ...
  [10:52:20.123457] Client #3 - Message 1: ...
  [10:52:20.123458] Client #2 - Message 1: ...
  [10:52:21.124567] Client #5 - Message 2: ...
  [10:52:21.124568] Client #1 - Message 2: ...

C'est la preuve que les clients sont traités en PARALLÈLE et non en séquence !


STRUCTURE DES FICHIERS
======================

Transfert de messages en mode connecté/
├── stationServeur_concurrent.c    # Serveur concurrent (threads)
├── stationClient_test.c           # Client pour tests
├── test_concurrent.sh             # Script de test automatisé
├── test_logs/                     # Répertoire des logs (créé par le script)
│   ├── server.log                 # Logs du serveur
│   ├── client_1.log               # Logs du client 1
│   ├── client_2.log               # Logs du client 2
│   └── ...
└── README.txt                     # Cette documentation


DÉTAILS TECHNIQUES
==================

Serveur Concurrent (stationServeur_concurrent.c)
----------------------------------------------

Caractéristiques:
  ✓ Utilise pthread_create() pour créer un thread par client
  ✓ Boucle infinie d'acceptation de connexions
  ✓ Mutex pour protéger le compteur de clients (thread-safe)
  ✓ Timestamps en microsecondes pour tracer les événements
  ✓ SO_REUSEADDR pour éviter les erreurs de port occupé

Fonctionnement:
  1. Crée un socket serveur et se met en écoute (port 8080)
  2. Accepte chaque nouvelle connexion client
  3. Crée un thread dédié pour gérer ce client
  4. Le thread reçoit 60 messages du client
  5. Le thread se ferme après transmission complète


Client (stationClient_test.c)
----------------------------

Caractéristiques:
  ✓ Se connecte au serveur sur 127.0.0.1:8080
  ✓ Envoie 60 messages avec timestamp
  ✓ Délai d'1 seconde entre les envois
  ✓ Identifiant unique (PID du processus)
  ✓ Logs détaillés avec microsecondes


TROUBLESHOOTING
===============

Port déjà utilisé
-----------------
Erreur: Address already in use

Solution: Attendre quelques secondes ou modifier le PORT dans le code 
(8080 → 8081, etc.)


Permission refusée sur le script
--------------------------------
Erreur: bash: ./test_concurrent.sh: Permission denied

Solution:
  chmod +x test_concurrent.sh


Erreur de compilation (pthread)
-------------------------------
Erreur: undefined reference to `pthread_create'

Solution: S'assurer que -pthread est dans la compilation:
  gcc -pthread -o stationServeur_concurrent stationServeur_concurrent.c


Les clients se disconnectent prématurément
------------------------------------------
Vérifier que le serveur est bien lancé et que le port 8080 est accessible.


CAS D'USAGE RÉELS
=================

Ce modèle concurrent est utilisé dans:
  - Serveurs web (Apache, Nginx avec threads)
  - Bases de données (PostgreSQL, MySQL)
  - Serveurs de chat en temps réel
  - Serveurs de jeux multijoueurs
  - APIs REST multi-clients


NOTES IMPORTANTES
=================

  - Le nombre de threads est limité par l'OS (généralement 1000-10000)
  - Les mutex garantissent l'accès sécurisé aux variables partagées
  - Les timestamps permettent de voir l'exact ordre d'exécution
  - Les logs peuvent être analysés pour déboguer les problèmes 
    de synchronisation
  - Le port par défaut est 8080 (modifiable dans le code)
  - Chaque client envoie 60 messages avec 1 seconde d'intervalle


CONTACT & DOCUMENTATION
=======================

Pour plus d'informations sur:
  - Les threads POSIX: man pthread
  - Les sockets TCP: man socket
  - La synchronisation: man pthread_mutex

Créé pour démontrer la programmation concurrent en C avec les sockets TCP 
et les threads POSIX.

================================================================================
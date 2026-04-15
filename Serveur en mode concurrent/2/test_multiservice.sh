#!/bin/bash

# Script de test pour serveur multi-service concurrent

LOG_DIR="./test_logs_multiservice"
SERVER_LOG="$LOG_DIR/server.log"

# Créer le répertoire de logs
mkdir -p "$LOG_DIR"
rm -f "$LOG_DIR"/*

echo "============================================"
echo "Test du Serveur Multi-Service Concurrent"
echo "============================================"
echo ""

# Compilation
echo "[1] Compilation du serveur multi-service..."
gcc -pthread -o stationServeur_multiservice stationServeur_multiservice.c 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Erreur de compilation du serveur!"
    exit 1
fi

echo "[2] Compilation des clients de test..."
gcc -o client_echo client_echo.c 2>/dev/null
gcc -o client_sysinfo client_sysinfo.c 2>/dev/null
gcc -o client_filelist client_filelist.c 2>/dev/null

if [ $? -ne 0 ]; then
    echo "Erreur de compilation des clients!"
    exit 1
fi

echo "[3] Lancement du serveur..."
./stationServeur_multiservice > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!
sleep 1

echo "[4] SCÉNARIO 1: Deux clients ECHO en parallèle"
echo "===================="
START_TIME=$(date +%s%N)

./client_echo > "$LOG_DIR/echo1.log" 2>&1 &
PID1=$!
sleep 0.5
./client_echo > "$LOG_DIR/echo2.log" 2>&1 &
PID2=$!

wait $PID1 $PID2
END_TIME=$(date +%s%N)
ELAPSED=$((($END_TIME - $START_TIME) / 1000000))

echo "Temps écoulé ECHO (2 clients): ${ELAPSED}ms"
echo ""

echo "[5] SCÉNARIO 2: Deux clients SYSINFO en parallèle"
echo "===================="
START_TIME=$(date +%s%N)

./client_sysinfo > "$LOG_DIR/sysinfo1.log" 2>&1 &
PID1=$!
sleep 0.5
./client_sysinfo > "$LOG_DIR/sysinfo2.log" 2>&1 &
PID2=$!

wait $PID1 $PID2
END_TIME=$(date +%s%N)
ELAPSED=$((($END_TIME - $START_TIME) / 1000000))

echo "Temps écoulé SYSINFO (2 clients): ${ELAPSED}ms"
echo ""

echo "[6] SCÉNARIO 3: Un client de chaque service en parallèle"
echo "===================="
START_TIME=$(date +%s%N)

./client_echo > "$LOG_DIR/echo_mixed.log" 2>&1 &
PID1=$!
sleep 0.5
./client_sysinfo > "$LOG_DIR/sysinfo_mixed.log" 2>&1 &
PID2=$!
sleep 0.5
./client_filelist > "$LOG_DIR/filelist_mixed.log" 2>&1 &
PID3=$!

wait $PID1 $PID2 $PID3
END_TIME=$(date +%s%N)
ELAPSED=$((($END_TIME - $START_TIME) / 1000000))

echo "Temps écoulé (3 services différents): ${ELAPSED}ms"
echo ""

# Terminer le serveur
kill $SERVER_PID 2>/dev/null
sleep 1

echo "============================================"
echo "RÉSULTATS DU TEST"
echo "============================================"
echo ""

echo "--- Logs du Serveur (premiers événements) ---"
head -30 "$SERVER_LOG"
echo ""
echo "--- Services traités en parallèle (chronologie) ---"
grep "Client #" "$SERVER_LOG" | head -20
echo ""

echo "--- Résumé des tests ---"
echo "Fichiers de logs créés:"
ls -lh "$LOG_DIR"
echo ""

echo "--- Vérification du parallélisme ---"
echo "ECHO Client 1:"
grep "connecté\|Déconnexion" "$LOG_DIR/echo1.log" | head -2
echo ""
echo "ECHO Client 2:"
grep "connecté\|Déconnexion" "$LOG_DIR/echo2.log" | head -2
echo ""
echo "SYSINFO Client 1:"
grep "connecté\|Déconnexion" "$LOG_DIR/sysinfo1.log" | head -2
echo ""
echo "SYSINFO Client 2:"
grep "connecté\|Déconnexion" "$LOG_DIR/sysinfo2.log" | head -2
echo ""

echo "============================================"
echo "Test terminé. Logs disponibles dans: $LOG_DIR/"
echo "============================================"
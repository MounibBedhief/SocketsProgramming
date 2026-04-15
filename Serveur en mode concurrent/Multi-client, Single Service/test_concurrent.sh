#!/bin/bash

# Script de test pour serveur concurrent multi-clients

NUM_CLIENTS=${1:-5}
LOG_DIR="./test_logs"
SERVER_LOG="$LOG_DIR/server.log"

# Créer le répertoire de logs
mkdir -p "$LOG_DIR"
rm -f "$LOG_DIR"/*

echo "============================================"
echo "Test du Serveur Concurrent"
echo "============================================"
echo "Nombre de clients: $NUM_CLIENTS"
echo "Port: 8080"
echo ""

# Compilation
echo "[1] Compilation du serveur concurrent..."
gcc -pthread -o stationServeur_concurrent stationServeur_concurrent.c
if [ $? -ne 0 ]; then
    echo "Erreur de compilation du serveur!"
    exit 1
fi
echo "✓ Serveur compilé"

echo ""
echo "[2] Compilation du client test..."
gcc -o stationClient_test stationClient_test.c
if [ $? -ne 0 ]; then
    echo "Erreur de compilation du client!"
    exit 1
fi
echo "✓ Client compilé"

echo ""
echo "[3] Lancement du serveur..."
./stationServeur_concurrent > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!
echo "✓ Serveur lancé (PID: $SERVER_PID)"
sleep 2

echo ""
echo "[4] Lancement de $NUM_CLIENTS clients simultanément..."
START_TIME=$(date +%s)

# Lancer les clients en parallèle
PIDS=()
for i in $(seq 1 $NUM_CLIENTS); do
    ./stationClient_test > "$LOG_DIR/client_$i.log" 2>&1 &
    PIDS+=($!)
    echo "  → Client $i lancé (PID: ${PIDS[-1]})"
done

echo ""
echo "En attente que tous les clients se terminent..."

# Attendre que TOUS les clients se terminent
for pid in "${PIDS[@]}"; do
    wait $pid
done

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo ""
echo "✓ Tous les clients terminés"
sleep 1

# Terminer le serveur
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "============================================"
echo "RÉSULTATS DU TEST"
echo "============================================"
echo ""
echo "Temps total d'exécution: ${ELAPSED}s"
echo "Temps attendu (si séquentiel): $((NUM_CLIENTS * 60))s"
if [ $ELAPSED -lt $((NUM_CLIENTS * 30)) ]; then
    echo "✓✓✓ PARALLÉLISME CONFIRMÉ! Messages traités en PARALLÈLE"
else
    echo "⚠ Execution semble séquentielle"
fi
echo ""

echo "--- Logs du Serveur (premiers messages reçus) ---"
head -40 "$SERVER_LOG"
echo ""

echo "--- Résumé des Clients ---"
TOTAL_MESSAGES=0
for i in $(seq 1 $NUM_CLIENTS); do
    if [ -f "$LOG_DIR/client_$i.log" ]; then
        messages=$(grep -c "Envoi du message" "$LOG_DIR/client_$i.log" 2>/dev/null || echo "0")
        echo "Client $i: $messages messages envoyés"
        TOTAL_MESSAGES=$((TOTAL_MESSAGES + messages))
    fi
done
echo ""
echo "TOTAL: $TOTAL_MESSAGES messages envoyés"

echo ""
echo "--- Analyse Parallélisme (entrecroisement des messages) ---"
echo "Messages du serveur par client (ordre reçu):"
grep "Client #" "$SERVER_LOG" | grep "Message" | head -20
echo ""

echo "Logs complets disponibles dans: $LOG_DIR/"
echo "============================================"
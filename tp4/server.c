#include "common.h"

// #include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
// #include <sys/time.h>
#include <string.h>
#include <unistd.h>
// #include <errno.h>

/**
 * ------------------------------------------------
 * == ABSTRACT ====================================
 * ------------------------------------------------
 */

typedef struct {
    int socket;
    int id;
} Equipment;

Equipment servGetEmptyEquipment() {
    Equipment client = { 0, 0 };
    return client;
}

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

void servDebugStep(const char* log);
void servThreadDebugStep(const char* log);
void servExplainAndDie(char **argv);
void servNotifySendingFailureAndDie(const int clientId);
void servDebugEquipmentsCount(void);

bool servValidateInput(int argc, char **argv);
void *servThreadClientHandler(void *data);
void servThreadCloseOnError(const Equipment *client, const char *errMsg);

Message servReceiveMsg(const int cliSocket);
void servUnicast(Message msg, Equipment client);
void servBroadcast(Message msg);

void servAddEquipment(const int clientSocket);
Equipment servGetEmptyEquipment();


/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int nEquipments = 0;
Equipment equipments[MAX_CONNECTIONS] = { {0}, {0} };

int main(int argc, char **argv) {

	// Validate initialization command
	servDebugStep("Validating input...");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    servDebugStep("Creating server socket...");
    const char *portStr = argv[1];
    const int ipVersion = 4;
    int servSocket = netListen(portStr, TIMEOUT_CONN_SECS, MAX_CONNECTIONS, &ipVersion);

    if (DEBUG_ENABLE) {
        char dbgTxt[BUF_SIZE] = "";
        char boundAddr[200] = "";
        if (!netSetSocketAddrString(servSocket, boundAddr)) {
            sprintf(dbgTxt, "Failure as trying to exhibit bound address...");
            comLogErrorAndDie(dbgTxt);
        }
        sprintf(dbgTxt, "All set! Server is bound to %s:%s\nWaiting for connections...", boundAddr, portStr);
        servDebugStep(dbgTxt);
    }

    // Accept & open thread to handle new client
    while (true) {
        int cliSocket = netAccept(servSocket);
        pthread_t threadID;
        Equipment clientData = servGetEmptyEquipment();
        clientData.socket = cliSocket;
        pthread_create(&threadID, NULL, servThreadClientHandler, &clientData);
    }

    exit(EXIT_SUCCESS);
}

void *servThreadClientHandler(void *threadData) {
    
    servThreadDebugStep("Starting new thread...");
    Equipment *client = (Equipment *)threadData;

    // Parse input
    if (DEBUG_ENABLE) {
        char clientAddrStr[INET6_ADDRSTRLEN + 1] = "";
        if (netSetSocketAddrString(client->socket, clientAddrStr)) {
            const char auxTemplate[] = "Connected to client at %s...";
			char *aux = (char *)malloc(strlen(clientAddrStr) + strlen(auxTemplate) + 1);
			sprintf(aux, auxTemplate, clientAddrStr);
            servThreadDebugStep(aux);
			free(aux);
        }
    }

    servThreadDebugStep("Waiting for messages...");
    while (true) {
        
        // Handle request
        Message msg = servReceiveMsg(client->socket);
        switch (msg.id) {
            case MSG_REQ_ADD:
                servThreadDebugStep("Someone is trying to connect...");
                servAddEquipment(client->socket);
                break;
        
            default:
                servThreadDebugStep("Something wrong isn't right...");
                break;
        }
    }

    // End thread successfully
	servThreadDebugStep("Done!");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

/* -- Debug ------------------ */

void servDebugStep(const char* log) {
    if (DEBUG_ENABLE) {
        const char prefix[] = "[serv]";
        char *aux = (char *)malloc(strlen(log) + strlen(prefix) + 2);
        sprintf(aux, "%s %s", prefix, log);
        comDebugStep(aux);
        free(aux);
    }
}

void servThreadDebugStep(const char* log) {
    if (DEBUG_ENABLE) {
        const char prefix[] = "[serv: thread]";
        char *aux = (char *)malloc(strlen(log) + strlen(prefix) + 2);
        sprintf(aux, "%s %s", prefix, log);
        comDebugStep(aux);
        free(aux);
    }
}

void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [port number]>\n", argv[0]);
	printf("Example: %s %d\n", argv[0], PORT_DEFAULT);
    exit(EXIT_FAILURE);
}

void servNotifySendingFailureAndDie(const int clientId) {
    const char auxTemplate[] = "[unicast] Failure as sending message to 'equipment %d'";
    char *aux = (char *)malloc(strlen(auxTemplate) + 1);
    sprintf(aux, auxTemplate, clientId);
    comLogErrorAndDie(aux);
}

int* getEquipIdList(const Equipment equips[], const int length) {
    int *eqIds = (int *)malloc(length * sizeof(int));
    for (int i = 0; i < length; i++) {
        eqIds[i] = equips[i].id;
    }
    return eqIds;
}

void servDebugEquipmentsCount() {
    
    if (!DEBUG_ENABLE)
        return;
    
    int *eqIds = getEquipIdList(equipments, nEquipments);
    const char *equipListString = strGetStringFromIntList(eqIds, nEquipments);
    const char auxTemplate[] = "'%d' equipment(s) currently: '%s'";
    char *aux = (char *)malloc(strlen(auxTemplate) + strlen(equipListString) + 1);
    sprintf(aux, auxTemplate, nEquipments, equipListString);
    
    servDebugStep(aux);
    free(aux);
}

/* -- Main ------------------- */

bool servValidateInput(int argc, char **argv) {

	if (argc != 2) {
        servDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[1];
	if (!strIsNumeric(portStr)) {
		servDebugStep("Invalid Port!\n");
		return false;
	}

	return true;
}

Message servReceiveMsg(const int cliSocket) {
    
    servDebugStep("Waiting for messages...");

    char buffer[BUF_SIZE] = "";
    ssize_t receivedBytes = netRecv(cliSocket, buffer, TIMEOUT_TRANSFER_SECS);
    if (receivedBytes == -1)
        comLogErrorAndDie("Failure as trying to receive messages from client");

    if (DEBUG_ENABLE) {
        const char auxTemplate[] = "Received buffer: '%s'";
        char *aux = (char *)malloc(strlen(auxTemplate) + strlen(buffer) + 2);
        sprintf(aux, "Received buffer: '%s'", buffer);
        servDebugStep(aux);
        free(aux);
    }

    Message msg = getEmptyMessage();
    setMessageFromText(buffer, &msg);

    if (!msg.isValid) {
        /**
         * TODO: 2022-06-26 - Handle this properly
         */
        servDebugStep("Invalid message received");
    }

    return msg;
}

void servUnicast(Message msg, Equipment client) {
    char buffer[BUF_SIZE] = "";
    if (!buildMessageToSend(msg, buffer, BUF_SIZE))
        comLogErrorAndDie("[unicast] Failure as trying to build message to send");
    if (!netSend(client.socket, buffer))
        servNotifySendingFailureAndDie(client.id);
}

void servBroadcast(Message msg) {

    // Build message
    char buffer[BUF_SIZE] = "";
    if (!buildMessageToSend(msg, buffer, BUF_SIZE))
        comLogErrorAndDie("[broadcast] Failure as trying to build message to send");

    // Send it to everyone
    for (int i = 0; i < nEquipments; i++) {
        Equipment client = equipments[i];
        if (!netSend(client.socket, buffer))
            servNotifySendingFailureAndDie(client.id);
    }
}

void servAddEquipment(const int clientSocket) {

    // Create new equipment
    Equipment newEquipment = servGetEmptyEquipment();
    newEquipment.socket = clientSocket;

    // Check if is there any room for it    
    if (nEquipments >= MAX_CONNECTIONS) {
        servDebugStep("Max equipments reached...");
        Message errorMsg = getEmptyMessage();
        errorMsg.id = MSG_ERR;
        errorMsg.payload = (int *)malloc(sizeof(int));
        *(int *)errorMsg.payload = ERR_MAX_EQUIP;
        servUnicast(errorMsg, newEquipment);
        return;
    }

    // Save current equip list
    const int prevNEquips = nEquipments;
    const int equipListSize = prevNEquips * sizeof(Equipment);
    Equipment *prevEquipList = (Equipment *)malloc(equipListSize);
    memcpy(prevEquipList, equipments, equipListSize);

    // Let it in
    newEquipment.id = nEquipments + 1;
    equipments[nEquipments++] = newEquipment;

    // Let everyone know
    servDebugStep("Sending 'add equipment' response...");
    Message newEquipMsg = getEmptyMessage();
    newEquipMsg.id = MSG_RES_ADD;
    newEquipMsg.payload = (int *)malloc(sizeof(int));
    *(int *)newEquipMsg.payload = newEquipment.id;
    servBroadcast(newEquipMsg);

    // Tell the new guy who is in the group
    if (prevNEquips > 0) {
        servDebugStep("Sending 'list equipments' response to the new guy...");
        Message equipListMsg = getEmptyMessage();
        int *prevEquipIds = getEquipIdList(prevEquipList, prevNEquips);
        equipListMsg.id = MSG_RES_LIST;
        equipListMsg.payload = (int *)malloc(prevNEquips);
        equipListMsg.payload = prevEquipIds;
        equipListMsg.payloadSize = prevNEquips;
        servUnicast(equipListMsg, newEquipment);
    }
    
    // Log
    printf("\nEquipment %d added\n", newEquipment.id); // NOTE: This really should be printed
    servDebugEquipmentsCount();
}

void serverCloseThreadOnError(const Equipment *client, const char *errMsg) {
	close(client->socket);
    perror(errMsg);
    puts("\nClosing thread because of failure... :(\n");
    pthread_exit(NULL);
}
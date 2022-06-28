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

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

/* -- Main ----------- */

void *servThreadClientHandler(void *threadData);
void servAddEquipment(const int clientSocket);
void servSendError(const ErrorCodeEnum error, Equipment equipment);
void servSendEquipList(const Equipment list[], const int count, const Equipment target);

/* -- Helper --------- */

Equipment servGetEmptyEquipment();
int* servGetEquipIdList(const Equipment equips[], const int length);

Message servReceiveMsg(const int cliSocket);
void servUnicast(Message msg, Equipment client);
void servBroadcast(Message msg);

/* -- I/O ------------ */

bool servValidateInput(int argc, char **argv);
void servDebugEquipmentsCount();
void servNotifySendingFailureAndDie(const int clientId);
void serverCloseThreadOnError(const Equipment *client, const char *errMsg);
void servExplainAndDie(char **argv);


/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int nEquipments = 0;
Equipment equipments[MAX_CONNECTIONS] = { {0}, {0} };

int main(int argc, char **argv) {

	// Validate initialization command
	comDbgStep("Validating input...");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    comDbgStep("Creating server socket...");
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
        comDbgStep(dbgTxt);
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
    
    comDbgStep("Starting thread for a new client...");
    Equipment *client = (Equipment *)threadData;

    // Parse input
    if (DEBUG_ENABLE) {
        char clientAddrStr[INET6_ADDRSTRLEN + 1] = "";
        if (netSetSocketAddrString(client->socket, clientAddrStr)) {
            const char auxTemplate[] = "Connected to client at %s...";
			char *aux = (char *)malloc(strlen(clientAddrStr) + strlen(auxTemplate) + 1);
			sprintf(aux, auxTemplate, clientAddrStr);
            comDbgStep(aux);
			free(aux);
        }
    }

    comDbgStep("Waiting for messages...");
    while (true) {
        
        // Handle request
        Message msg = servReceiveMsg(client->socket);
        if (!msg.isValid) {
            comDbgStep("Invalid message received");
            continue;
        }

        switch (msg.id) {
            case MSG_REQ_ADD:
                comDbgStep("Someone is trying to connect...");
                servAddEquipment(client->socket);
                break;
        
            default:
                /**
                 * TODO: 2022-06-28 - Should we really print this?
                 */
                comDbgStep("Something wrong isn't right...");
                break;
        }
    }

    // End thread successfully
	comDbgStep("Done!");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}

void servAddEquipment(const int clientSocket) {

    // Create new equipment
    Equipment newEquipment = servGetEmptyEquipment();
    newEquipment.socket = clientSocket;

    // Check if is there any room for it    
    if (nEquipments >= MAX_CONNECTIONS) {
        comDbgStep("Max equipments reached...");
        servSendError(ERR_MAX_EQUIP, newEquipment);
        return;
    }

    // Save current equip list
    const int prevEquipCount = nEquipments;
    const int size = prevEquipCount * sizeof(Equipment);
    Equipment *prevEquipList = (Equipment *)malloc(size);
    memcpy(prevEquipList, equipments, size);

    // Let the new guy in
    const int i = nEquipments;
    newEquipment.id = getEquipIdFromIndex(i);
    equipments[i] = newEquipment;
    nEquipments++;

    // Let everyone know
    comDbgStep("Sending 'add equipment' response...");
    Message newEquipMsg = getEmptyMessage();
    newEquipMsg.id = MSG_RES_ADD;
    newEquipMsg.payload = strIntToString(newEquipment.id);
    servBroadcast(newEquipMsg);

    // Tell the new guy who's already in the group
    if (prevEquipCount > 0) {
        comDbgStep("Sending 'list equipments' response to the new guy...");
        servSendEquipList(prevEquipList, prevEquipCount, newEquipment);
    }
    
    // Log
    printf("\nEquipment %d added\n", newEquipment.id); // NOTE: This really should be printed
    servDebugEquipmentsCount();
}

void servSendEquipList(const Equipment list[], const int count, const Equipment target) {
    Message msg = getEmptyMessage();
    int *ids = servGetEquipIdList(list, count);
    msg.id = MSG_RES_LIST;
    msg.payload = strGetStringFromIntList(ids, count);
    servUnicast(msg, target);
}

void servSendError(const ErrorCodeEnum error, Equipment equipment) {
    Message errorMsg = getEmptyMessage();
    errorMsg.id = MSG_ERR;
    errorMsg.target = equipment.id;
    errorMsg.payload = strIntToString(error);
    servUnicast(errorMsg, equipment);
}

/**
 * ------------------------------------------------
 * == HELPER ======================================
 * ------------------------------------------------
 */

Equipment servGetEmptyEquipment() {
    Equipment equipment = { 0, 0 };
    return equipment;
}

int* servGetEquipIdList(const Equipment equips[], const int length) {
    int *eqIds = (int *)malloc(length * sizeof(int));
    for (int i = 0; i < length; i++) {
        eqIds[i] = equips[i].id;
    }
    return eqIds;
}

Message servReceiveMsg(const int cliSocket) {

    char buffer[BUF_SIZE] = "";
    ssize_t receivedBytes = netRecv(cliSocket, buffer, TIMEOUT_TRANSFER_SECS);
    if (receivedBytes == -1)
        comLogErrorAndDie("Failure as trying to receive messages from client");

    if (DEBUG_ENABLE) {
        const char auxTemplate[] = "Received buffer: '%s'";
        char *aux = (char *)malloc(strlen(auxTemplate) + strlen(buffer) + 2);
        sprintf(aux, auxTemplate, buffer);
        comDbgStep(aux);
        free(aux);
    }

    Message msg = getEmptyMessage();
    setMessageFromText(buffer, &msg);
	if (!msg.isValid) {
		comDbgStep("Invalid message received...");
		comDebugProtocolMessage(msg);
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

/**
 * ------------------------------------------------
 * == I/O =========================================
 * ------------------------------------------------
 */

void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [port number]>\n", argv[0]);
	printf("Example: %s %d\n", argv[0], PORT_DEFAULT);
    exit(EXIT_FAILURE);
}

void serverCloseThreadOnError(const Equipment *client, const char *errMsg) {
	close(client->socket);
    perror(errMsg);
    puts("\nClosing thread because of failure... :(\n");
    pthread_exit(NULL);
}

void servNotifySendingFailureAndDie(const int clientId) {
    const char auxTemplate[] = "[unicast] Failure as sending message to 'equipment %d'";
    char *aux = (char *)malloc(strlen(auxTemplate) + 1);
    sprintf(aux, auxTemplate, clientId);
    comLogErrorAndDie(aux);
    free(aux);
}

void servDebugEquipmentsCount() {
    
    if (!DEBUG_ENABLE)
        return;
    
    int *eqIds = servGetEquipIdList(equipments, nEquipments);
    const char *equipListString = strGetStringFromIntList(eqIds, nEquipments);
    const char auxTemplate[] = "'%d' equipment(s) currently: '%s'";
    char *aux = (char *)malloc(strlen(auxTemplate) + strlen(equipListString) + 1);
    sprintf(aux, auxTemplate, nEquipments, equipListString);
    
    comDbgStep(aux);
    free(aux);
}

bool servValidateInput(int argc, char **argv) {

	if (argc != 2) {
        comDbgStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[1];
    const bool isIntOnly = true;
	if (!strIsNumeric(portStr, &isIntOnly)) {
		comDbgStep("Invalid Port!\n");
		return false;
	}

	return true;
}
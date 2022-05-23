#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv);
void servExplainAndDie(char **argv);

void servReceiveMsg(const int servSocket, int cliSocket, char buffer[BUF_SIZE]);
void servAddSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servListSensors(Command cmd, const Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servReadSensor(Command cmd, const Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servRemoveSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servExecuteCommand(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);

void handleError(const ErrCodeEnum error, char* input, char* answer, bool* mustFinish);
int servGetSensorsCount(const Equipment equipments[EQUIP_COUNT]);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int main(int argc, char **argv) {

    // Validate initialization command
	comDebugStep("Validating input...");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    comDebugStep("Creating server socket...\n");
    const int port = atoi(argv[2]);
    int servSocket = netListen(port, TIMEOUT_CONN_SECS, MAX_CONNECTIONS);

    const int dbgTxtLength = BUF_SIZE;
    char dbgTxt[dbgTxtLength];
    
    if (DEBUG_ENABLE) {

        char boundAddr[200];
        memset(dbgTxt, 0, dbgTxtLength);
        if (!netSetSocketAddressString(servSocket, boundAddr)) {
            sprintf(dbgTxt, "\nFailure as trying to exhibit bound address...\n");
            comLogErrorAndDie(dbgTxt);
        }

        sprintf(dbgTxt, "\nAll set! Server is bound to %s:%d\nWaiting for connections...\n", boundAddr, port);
        comDebugStep(dbgTxt);
    }

    // Initialize equipments list
    Equipment equipments[EQUIP_COUNT];
    for (int i = 0; i < EQUIP_COUNT; i++)
        equipments[i] = getEmptyEquipment(i);

    // Accept client
    int cliSocket = netAccept(servSocket);

    while (true) {
        
        // Receive message
        char input[BUF_SIZE];
        memset(input, 0, BUF_SIZE);
        servReceiveMsg(servSocket, cliSocket, input);

        // Parse command
        char answer[BUF_SIZE];
        memset(answer, 0, BUF_SIZE);
        const Command cmd = getCommand(input);
        bool shouldSendResponse = true;
        
        // Execute command
        if (!cmd.error) {
            if (DEBUG_ENABLE) {
                memset(dbgTxt, 0, dbgTxtLength);
                sprintf(dbgTxt, "Command detected: [%d] '%s'", cmd.code, CMD_NAME[cmd.code]);
                comDebugStep(dbgTxt);
            }
            servExecuteCommand(cmd, equipments, answer);
        }
        
        // Handle error
        if (cmd.error) {
            handleError(cmd.error, input, answer, &shouldSendResponse);
            memset(dbgTxt, 0, dbgTxtLength);
            sprintf(dbgTxt, "Error!\n\t'%s'", answer);
            comDebugStep(dbgTxt);
        }

        // Send response
        if (shouldSendResponse) {
            comDebugStep("Sending response..");
            const bool isSuccess = netSend(cliSocket, answer);
            if (!isSuccess)
                comLogErrorAndDie("Sending command failure");
            comDebugStep("Response sent!");
        }

        // End connection
        bool mustFinish = !shouldSendResponse || cmd.code == CMD_CODE_KILL;
        if (mustFinish)
            break;
    }

    // Finish...
	comDebugStep("Closing connection...");
    comDebugStep("\n --- THE END --- \n");
    close(servSocket);
    close(cliSocket);
    exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv) {

	if (argc != 3) {
        comDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate ip type
	if (netGetIpType(argv[1]) == -1) {
		comDebugStep("Invalid IP type!\n");
		return false;
	}

    // Validate port
	const char *portStr = argv[1];
	if (!strValidateNumeric(portStr, strlen(portStr))) {
		comDebugStep("Invalid Port!\n");
		return false;
	}

	return true;
}

void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [ip type] [port number]>\n", argv[0]);
	printf("Example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void servReceiveMsg(const int servSocket, const int cliSocket, char buffer[BUF_SIZE]) {
    
    comDebugStep("Waiting for command...\n");
    memset(buffer, 0, BUF_SIZE);
    
    size_t receivedBytes = netRecv(cliSocket, buffer, TIMEOUT_TRANSFER_SECS);
    if (receivedBytes == -1)
        comLogErrorAndDie("Failure as trying to receive messages from client");

    if (DEBUG_ENABLE) {
        char aux[BUF_SIZE];
        sprintf(aux, "Received buffer: '%s'", buffer);
        comDebugStep(aux);
    }
}

void handleError(const ErrCodeEnum error, char* input, char* answer, bool* shouldSendResponse) {
    switch (error) {
        case ERR_EQUIP_INVALID:
            *shouldSendResponse = true;
            sprintf(answer, "invalid equipment");
            break;
        case ERR_SENSOR_INVALID:
            *shouldSendResponse = true;
            sprintf(answer, "invalid sensor");
            break;
        case ERR_SENSOR_LIMIT:
            *shouldSendResponse = true;
            sprintf(answer, "limit exceeded");
            break;
        case ERR_CMD_INVALID:
        case ERR_SENSOR_REPEATED:
        default:
            *shouldSendResponse = false;
            sprintf(answer, "invalid command received: '%s'", input);
            break;
    }
}

bool servExecuteCommand(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]) {
    switch (cmd.code) {
        case CMD_CODE_ADD:
            return servAddSensor(cmd, allEquipments, answer);
        case CMD_CODE_LIST:
            return servListSensors(cmd, allEquipments, answer);
        case CMD_CODE_READ:
            return servReadSensor(cmd, allEquipments, answer);
        case CMD_CODE_RM:
            return servRemoveSensor(cmd, allEquipments, answer);
        case CMD_CODE_KILL:
        default:
            strcpy(answer, CMD_NAME[CMD_CODE_KILL]);
            break;
    }
}

void servAddSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char* answer) {

    // Parse command
    Equipment* equip = &allEquipments[cmd.equipCode];

    int addedSensors = 0;
    bool hasRepeated = false;
    bool addedList[SENSOR_COUNT] = { false };
    bool repeatedList[SENSOR_COUNT] = { false };

    int currentCount = servGetSensorsCount(allEquipments);

    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        if (!cmd.sensors[i])
            continue;

        // Compute this adding try
        if (equip->sensors[i]) {
            hasRepeated = true;
            repeatedList[i] = true;
            continue;
        }

        addedSensors++;
        if (currentCount + addedSensors > MAX_SENSORS) {
            cmd.error = ERR_SENSOR_LIMIT;
            return;
        }

        addedList[i] = true;
    }

    // Build answer message
    memset(answer, 0, BUF_SIZE);
    strcpy(answer, "sensor ");

    if (addedSensors) {
        for (int i = 0; i < SENSOR_COUNT; i++) {
            if (addedList[i]) {
                equip->sensors[i] = true;
                strcat(answer, SENSOR_IDS[i]);
                strcat(answer, " ");
            }
        }
        strcat(answer, "added ");
    }

    if (hasRepeated) {
        for (int i = 0; i < SENSOR_COUNT; i++) {
            if (repeatedList[i]) {
                strcat(answer, SENSOR_IDS[i]);
                strcat(answer, " ");
            }
        }
        strcat(answer, "already exists in");
        strcat(answer, EQUIP_IDS[equip->code]);
    }
}

void servRemoveSensor(const Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]) {
    
    bool hasRemoved = false;
    bool hasMissed = false;
    bool removedList[SENSOR_COUNT] = { false };
    bool missedList[SENSOR_COUNT] = { false };

    // Parse command
    Equipment* equip = &allEquipments[cmd.equipCode];
    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        if (!cmd.sensors[i])
            continue;

        if (equip->sensors[i]) {
            hasRemoved = true;
            removedList[i] = true;
        } else {
            hasMissed = true;
            missedList[i] = true;
        }
    }

    // Build answer
    memset(answer, 0, BUF_SIZE);
    strcpy(answer, "sensor ");

    if (hasRemoved) {
        for (int i = 0; i < SENSOR_COUNT; i++) {
            if (removedList[i]) {
                equip->sensors[i] = false;
                strcat(answer, SENSOR_IDS[i]);
                strcat(answer, " ");
            }
        }
        strcat(answer, "removed ");
    }

    if (hasMissed) {
        for (int i = 0; i < SENSOR_COUNT; i++) {
            if (missedList[i]) {
                strcat(answer, SENSOR_IDS[i]);
                strcat(answer, " ");
            }
        }
        strcat(answer, "does not exist in ");
        strcat(answer, EQUIP_IDS[equip->code]);
    }
}

void servReadSensor(const Command cmd, const Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]) {

    // Seed for random values
    srand(time(NULL));

    // Prepare answer string
    int tempLength = 10;
    char tempMsg[tempLength];
    memset(answer, 0, BUF_SIZE);

    // Parse requested values
    bool hasValidSensors = false;
    bool hasInvalidSensors = false;
    int notInstalledSensors[SENSOR_COUNT] = { false };

    Equipment equip = allEquipments[cmd.equipCode];
    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        // Check applicability
        if (!cmd.sensors[i])
            continue;

        if (!equip.sensors[i]) {
            hasInvalidSensors = true;
            notInstalledSensors[i] = true;
            continue;
        }

        // Compute reading values
        hasValidSensors = true;
        float reading = rand() % 100;
        notInstalledSensors[i] = false;
        
        memset(tempMsg, 0, tempLength);
        sprintf(tempMsg, "%.2f ", reading);
        strcat(answer, tempMsg);
    }

    // Handle not installed requested values
    if (hasInvalidSensors) {

        if (hasValidSensors)
            strcat(answer, "and ");

        for (int i = 0; i < SENSOR_COUNT; i++) {
            if (notInstalledSensors[i]) {
                strcat(answer, SENSOR_IDS[i]);
                strcat(answer, " ");
            }
        }
        strcat(answer, "not installed");
    }
}

void servListSensors(const Command cmd, const Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]) {

    Equipment equip = allEquipments[cmd.equipCode];
    memset(answer, 0, BUF_SIZE);
    bool isThereAny = false;
    
    for (int i = 0; i < SENSOR_COUNT; i++) {
        if (equip.sensors[i]) {
            isThereAny = true;
            strcat(answer, SENSOR_IDS[i]);
            strcat(answer, " ");
        }
    }

    if (!isThereAny)
        strcpy(answer, "none");
}

int servGetSensorsCount(const Equipment equipments[EQUIP_COUNT]) {
    int count = 0;
    for (int i = 0; i < EQUIP_COUNT; i++) {
        for (int j = 0; j < SENSOR_COUNT; j++) {
            if (equipments[i].sensors[j])
                count++;
        }   
    }
    return count;
}

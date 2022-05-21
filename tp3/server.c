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

void servAddSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servListSensors(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servReadSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servRemoveSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);
void servExecuteCommand(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);

int servGetSensorsCount(const Equipment equipments[EQUIP_COUNT]);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int main(int argc, char **argv) {

    // Validate initialization command
	comDebugStep("Validating input...\n");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    comDebugStep("Creating server socket...\n");
    const int port = atoi(argv[2]);
    int servSocket = netListen(port, TIMEOUT_CONN_SECS, MAX_CONNECTIONS);

    const int dbgTxtLength = 500;
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

    // Seed for random values
    srand(time(NULL));

    // Initialize equipments list
    Equipment equipments[EQUIP_COUNT];
    for (int i; i < EQUIP_COUNT; i++)
        equipments[i] = getEmptyEquipment(EQUIP_IDS[i]);

    // Listen for messages
    while (true) {
        
        // Receive messages
        char input[BUF_SIZE];
        memset(input, 0, BUF_SIZE);
        const int cliSocket = netAccept(servSocket);
        size_t receivedBytes = netRecv(cliSocket, input, TIMEOUT_TRANSFER_SECS);
        if (receivedBytes == -1)
            comLogErrorAndDie("Failure as trying to receive messages from client");

        // Determine command
        input[BUF_SIZE - 1] = '\n';
        const Command cmd = getCommand(input);
        printf("\n Command receivedBytes: %d | cmd.code / name = '%d' / '%s'", receivedBytes, cmd.code, cmd.name);
        
        // Validate message
        if (!cmd.isValid) {
            memset(dbgTxt, 0, dbgTxtLength);
            sprintf(dbgTxt, "\nInvalid command received: '%s'", input);
            comLogErrorAndDie(dbgTxt);
        }

        // Handle command
        char answer[BUF_SIZE];
        servExecuteCommand(cmd, equipments, answer);
        break;
    }
    

    // Determine command
        // Add sensor
        // Read sensor
        // Remove sensor
        // List equipment sensors
    // Run command
    // Send response
    // Close connection
    // End execution

    // Finish...
	comDebugStep("\nClosing connection...\n");
    close(servSocket);
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

void servAddSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char* answer) {
    
    // Prepare answer string
    int tempLength = 100;
    char tempMsg[tempLength];
    memset(tempMsg, 0, tempLength);
    
    memset(answer, 0, BUF_SIZE);
    strcpy(answer, "sensor ");

    Equipment equip = allEquipments[cmd.equipCode];
    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        if (!cmd.sensors[i])
            continue;

        // Verify limit exceeding
        if (servGetSensorsCount(allEquipments) >= MAX_SENSORS) {
            memset(tempMsg, 0, tempLength);
            strcpy(answer, "limit exceeded");
            return;
        }

        // Try to add
        int sensorId = SENSOR_IDS[i];
        if (!equip.sensors[i]) {
            equip.sensors[i] = true;
            sprintf(tempMsg, "%s added ", sensorId);
        } else
            sprintf(tempMsg, "%s already exists ", sensorId);

        strcat(answer, tempMsg);
    }
}

void servRemoveSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char* answer) {
    
    // Prepare answer string
    int tempLength = 100;
    char tempMsg[tempLength];
    memset(tempMsg, 0, tempLength);
    
    memset(answer, 0, BUF_SIZE);
    strcpy(answer, "sensor ");

    // Remove em'all
    Equipment equip = allEquipments[cmd.equipCode];
    char equipId = EQUIP_IDS[cmd.equipCode];

    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        if (!cmd.sensors[i])
            continue;

        int sensorId = SENSOR_IDS[i];
        if (equip.sensors[i]) {
            equip.sensors[i] = false;
            sprintf(tempMsg, "%s removed ", sensorId);
        } else
            sprintf(tempMsg, "%s does not exist in %s ", sensorId, equipId);

        strcat(answer, tempMsg);
    }
}

void servReadSensor(Command cmd, Equipment allEquipments[EQUIP_COUNT], char* answer) {
    
    // Prepare answer string
    int tempLength = 100;
    char tempMsg[tempLength];

    int notInstalledSensors[SENSOR_COUNT];
    Equipment equip = allEquipments[cmd.equipCode];

    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        // Check applicability
        if (!cmd.sensors[i])
            continue;

        if (!equip.sensors[i]) {
            notInstalledSensors[i] = true;
            continue;
        }

        // Compute reading value
        float reading = rand() % 100;
        notInstalledSensors[i] = false;
        
        memset(tempMsg, 0, tempLength);
        sprintf(tempMsg, "%.2f ", reading);
        strcat(answer, tempMsg);
    }

    // Handle not installed requested values
    bool aux = false;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        
        if (!notInstalledSensors[i])
            continue;

        memset(tempMsg, 0, tempLength);
        if (!aux) {
            aux = true;
            sprintf(tempMsg, "and ");
        }

        sprintf(tempMsg, "%s%s ", tempMsg, SENSOR_IDS[i]);
        strcat(answer, tempMsg);
    }

    if (aux)
        strcat(answer, "not installed");
}

void servListSensors(Command cmd, Equipment allEquipments[EQUIP_COUNT], char* answer) {
    
    int tempLength = 100;
    char tempMsg[tempLength];

    Equipment equip = allEquipments[cmd.equipCode];
    for (int i = 0; i < SENSOR_COUNT; i++) {
        if (equip.sensors[i]) {
            memset(tempMsg, 0, tempLength);
            sprintf(tempMsg, "%d ", SENSOR_IDS[i]);
            strcat(answer, tempMsg);
        }
    }
}

void servExecuteCommand(Command cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]) {
    switch (cmd.code) {
        case CMD_CODE_ADD:
            servAddSensor(cmd, allEquipments, answer);
            break;
        case CMD_CODE_LIST:
            servListSensors(cmd, allEquipments, answer);
        case CMD_CODE_READ:
            servReadSensor(cmd, allEquipments, answer);
            break;
        case CMD_CODE_RM:
            servRemoveSensor(cmd, allEquipments, answer);
            break;
        case CMD_CODE_KILL:
        default:
            strcpy(answer, CMD_NAME[CMD_CODE_KILL]);
            break;
    }
}

int servGetSensorsCount(const Equipment equipments[]) {
    int count = 0;
    for (int i = 0; i < EQUIP_COUNT; i++) {
        for (int j = 0; i < SENSOR_COUNT; j++) {
            if (equipments[i].sensors[j])
                count++;
        }   
    }
    return count;
}

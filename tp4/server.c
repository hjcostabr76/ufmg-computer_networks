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

struct ClientData {
    int socket;
};

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv);
void servExplainAndDie(char **argv);

void servReceiveMsg(int cliSocket, char buffer[BUF_SIZE]);
// void servExecuteCommand(Command* cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);

// void handleError(const ErrCodeEnum error, char* input, char* answer, bool* mustFinish);

void *servThreadClientHandler(void *data);
void servThreadCloseOnError(const struct ClientData *client, const char *errMsg);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int nEquipments = 0;
bool equipments[MAX_CONNECTIONS] = { false };

// const char* NET_TAG_MSG = "<msg>";
const char* NET_TAG_ID = "<id>";
const char* NET_TAG_SRC = "<src>";
const char* NET_TAG_TARGET = "<target>";
const char* NET_TAG_PAYLOAD = "<payload>";

void buildMessageToSend(const Message msg, char *buffer) {
	memset(buffer, 0, BUF_SIZE);

    char aux = '\0';
	strcat(buffer, NET_TAG_MSG); // Message

    strcat(buffer, NET_TAG_ID); // Id
    aux = msg.id + '0';
    strcat(buffer, &aux);
	strcat(buffer, NET_TAG_ID); // Id
	
	strcat(buffer, NET_TAG_SRC); // Src
    aux = msg.source + '0';
    strcat(buffer, &aux);
	strcat(buffer, NET_TAG_SRC); // Src
	
	strcat(buffer, NET_TAG_TARGET); // Target
    aux = msg.target + '0';
    strcat(buffer, &aux);
	strcat(buffer, NET_TAG_TARGET); // Target
	
	strcat(buffer, NET_TAG_PAYLOAD); // Payload
    strcat(buffer, (char *)msg.payload);
	strcat(buffer, NET_TAG_PAYLOAD); // Payload

	strcat(buffer, NET_TAG_MSG); // Message
}


bool isProtocolInitialized = false;
char protocolPattern[150] = "";

void initProtocolMessagesValidator() {

    if (isProtocolInitialized)
        return;

    char patternPayload[] = "(<payload>[a-zA-Z][0-9a-zA-Z\\t ]+[a-zA-Z]<payload>)?";
    char patternHeader[] = "<id>[0-9]<id><src>[0-9]{1,2}<src><target>[0-9]{1,2}<target>";
    
    strcat(protocolPattern, NET_TAG_MSG);
    strcat(protocolPattern, patternHeader);
    strcat(protocolPattern, patternPayload);
    strcat(protocolPattern, NET_TAG_MSG);

    isProtocolInitialized = true;
}

bool validateReceivedMsg(const char *message) {
    initProtocolMessagesValidator();
    return strRegexMatch(protocolPattern, message, NULL);
}

void testBatch(const char **messages, const int nTests, const bool isValidMessage) {

    char testType[15] = "";
    strcpy(testType, isValidMessage ? "Good\0" : "Bad\0");
    
    for (int i = 0; i < nTests; i++) {
        
        printf("\nTesting %s pattern:\n\t\"%s\"...", testType, messages[i]);
        const bool passedValidation = validateReceivedMsg(messages[i]);
        const bool isSuccess = (passedValidation && isValidMessage) || (!passedValidation && !isValidMessage);
        
        char resultMsg[10];
        strcpy(resultMsg, isSuccess ? "OK" : "FAILED!");
        printf("\n\tTest %s", resultMsg);
    }

    printf("\n");
}

void testProtocolValidation() {

    printf("\n\n>>>>>>>>>> Tests for regex >>>>>>>>>>\n\n");

    /* ---------------- Good ----------------  */
    bool isValid = true;
    
    int nTests = 2;
    const char *goodMessages[] = {
        "<msg><id>1<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    testBatch(goodMessages, nTests, isValid);
    printf("\n");


    /* - Bad: Wrong Tag names ---------------  */
    isValid = false;
    
    nTests = 6;
    const char *badMessagesWrongTag[] = {
        "<message><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><message>",
        "<msg><id>1<id><source>2<source><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg/>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload/><msg>",
        "<msg><id>1<id><src>2<src><target>3<target/><payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<src/><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    testBatch(badMessagesWrongTag, nTests, isValid);
    printf("\n");


    /* - Bad: Missing close tags ------------  */
    nTests = 6;
    const char *badMessagesClosingTags[] = {
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<msg>",
        "<msg><id>1<id><src>2<src><target>3<target>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    testBatch(badMessagesClosingTags, nTests, isValid);
    printf("\n");

    /* - Bad: Missing required fields -------  */
    nTests = 4;
    const char *badMessagesMissingFields[] = {
        "<msg><id><id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src><src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target><target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload><payload><msg>",
    };
    testBatch(badMessagesMissingFields, nTests, isValid);
    printf("\n");

    /* - Bad: Extra characters --------------  */
    nTests = 5;
    const char *badMessagesExtraChars[] = {
        "<msg><id>123<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>123<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>123<target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun Dolur2<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>1Loren Ipsun Dolur2<payload><msg>"
    };
    testBatch(badMessagesMissingFields, nTests, isValid);
    printf("\n");

    /* - Bad: Invalid characters ------------  */
    nTests = 3;
    const char *badMessagesInvalidChars[] = {
        "<msg><id>1a<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>2b<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3c<target><msg>"
    };
    testBatch(badMessagesInvalidChars, nTests, isValid);
    printf("\n");


    /* - Bad: Unexpected spaces -------------  */
    nTests = 3;
    const char *badMessagesSpaces[] = {
        "<msg><id>1 <id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src> <src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3 <target><msg>"
    };
    testBatch(badMessagesSpaces, nTests, isValid);
    printf("\n");
}



// char buffer[BUF_SIZE] = "";
// char payload[MAX_PAYLOAD_SIZE] = "Loren Ipsun Dolur";
// const Message message = { 1, 2, 3, (void *)payload };

// buildMessageToSend(message, buffer);
// printf("\nBuilt message: '%s'\n", buffer);

// testProtocolValidation();

int main(int argc, char **argv) {

	// Validate initialization command
	comDebugStep("Validating input...");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    comDebugStep("Creating server socket...");
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
        comDebugStep(dbgTxt);
    }

    // Accept & open thread to handle new client
    while (true) {
        int cliSocket = netAccept(servSocket);
        pthread_t threadID;
        struct ClientData clientData = { cliSocket };
        pthread_create(&threadID, NULL, servThreadClientHandler, &clientData);
    }

    exit(EXIT_SUCCESS);
}

/**
 * TODO: 2021-06-07 - ADD Descricao
 */
void *servThreadClientHandler(void *threadInput) {

    comDebugStep("[thread] Starting new thread...");
    char notificationMsg[BUF_SIZE];
    
    // Parse input
    struct ClientData *client = (struct ClientData *)threadInput;

    if (DEBUG_ENABLE) {
        char clientAddrStr[INET6_ADDRSTRLEN + 1] = "";
        if (netSetSocketAddrString(client->socket, clientAddrStr)) {
            memset(notificationMsg, 0, BUF_SIZE);
            sprintf(notificationMsg, "[thread] Connected to client at %s...", clientAddrStr);
            comDebugStep(notificationMsg);
        }
    }

    // RECV
    char buffer[BUF_SIZE];
    comDebugStep("[thread] Receiving text length...");
    servReceiveMsg(client->socket, buffer);

    // SEND
    char answer[1000];
    netSend(client->socket, answer);

    // End thread successfully
	comDebugStep("[thread] Done!");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv) {

	if (argc != 2) {
        comDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[1];
	if (!strIsNumeric(portStr)) {
		comDebugStep("Invalid Port!\n");
		return false;
	}

	return true;
}

void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [port number]>\n", argv[0]);
	printf("Example: %s %d\n", argv[0], PORT_DEFAULT);
    exit(EXIT_FAILURE);
}

void servReceiveMsg(const int cliSocket, char buffer[BUF_SIZE]) {
    
    comDebugStep("Waiting for command...");
    memset(buffer, 0, BUF_SIZE);
    
    ssize_t receivedBytes = netRecv(cliSocket, buffer, TIMEOUT_TRANSFER_SECS);
    if (receivedBytes == -1)
        comLogErrorAndDie("Failure as trying to receive messages from client");

    if (DEBUG_ENABLE) {
        char aux[BUF_SIZE];
        sprintf(aux, "Received buffer: '%s'", buffer);
        comDebugStep(aux);
    }
}

void serverCloseThreadOnError(const struct ClientData *client, const char *errMsg) {
	close(client->socket);
    perror(errMsg);
    puts("\nClosing thread because of failure... :(\n");
    pthread_exit(NULL);
}
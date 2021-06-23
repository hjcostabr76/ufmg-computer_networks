import sys
import socket

'''
    TODO: 2021-06-22 - Mesclar cliente & servidor no arquivo principal unico
'''

if __name__ != "__main__":
    sys.exit()

# Declarar constantes
ENABLE_DEBUG = True
MAX_CONNECTIONS = 5
IP_ADDR = '127.0.0.1'

'''
    Funcao auxiliar para depuracao.
'''
def printDebug(dbtText):
    if ENABLE_DEBUG:
        print(dbtText)

# python3 dcc023c2 -s <port> <input> <output>
# python3 dcc023c2 -s 2000 <input> <output>
printDebug('Inicializando servidor...')

port = int(sys.argv[2])
# input = int(sys.argv[3])
# output = int(sys.argv[4])

try:

    # Criar servidor
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Escutar conexoes de clientes
    server.bind((IP_ADDR, port))
    server.listen(MAX_CONNECTIONS)

    while True:
        client, address = server.accept()
        printDebug(f"Connection Established - {address[0]}:{address[1]}")
        break

    # Encerrar
    server.close()

except:
    print("Falha Inesperada:", sys.exc_info()[0])

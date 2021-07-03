import sys
import socket
from typing import final

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

try:

    # Ler entrada
    port = int(sys.argv[2])
    input = str(sys.argv[3])
    output = str(sys.argv[4])

    # Abrir arquivos
    inputFD = open(input, "r")
    outputFD = open(output, "w")
    # printDebug(inputFD.read())

    # Criar servidor
    serverFD = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serverFD.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Escutar conexoes de clientes
    serverFD.bind((IP_ADDR, port))
    serverFD.listen(MAX_CONNECTIONS)

    while True:
        
        clientFD, address = serverFD.accept()
        printDebug(f"Connection Established - {address[0]}")


        break

except:
    print("Falha Inesperada:", sys.exc_info())

finally:
    serverFD.close()
    inputFD.close()
    outputFD.close()
import sys
import socket

'''
    TODO: 2021-06-22 - Mesclar cliente & servidor no arquivo principal unico
'''

if __name__ != "__main__":
    sys.exit()

# Declarar constantes
ENABLE_DEBUG = True

'''
    Funcao auxiliar para depuracao.
'''
def printDebug(dbtText):
    if ENABLE_DEBUG:
        print(dbtText)

# python3 dcc023c2 -c <IP> <port> <input> <output>
# python3 client.py -c 127.0.0.1 2000 input output
printDebug('Executando cliente...')

ip = str(sys.argv[2])
port = int(sys.argv[3])
# input = int(sys.argv[4])
# output = int(sys.argv[5])

try:

    # Criar cliente
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((ip, port))

except:
    print("Falha:", sys.exc_info())

finally:
    client.close()

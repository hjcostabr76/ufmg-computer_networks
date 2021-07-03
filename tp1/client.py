import sys
import socket

'''
    TODO: 2021-06-22 - Mesclar cliente & servidor no arquivo principal unico
'''

if __name__ != "__main__":
    sys.exit()

# Declarar constantes
ENABLE_DEBUG = True
MSG_LENGTH = 10

'''
    Funcao auxiliar para depuracao.
'''
def printDebug(dbtText):
    if ENABLE_DEBUG:
        print(dbtText)

def encode16(raw_text):
    return bytes(raw_text, 'utf-8').hex()

'''
    TODO: 2021-06-24 - Implementar...
'''
def decode16(encoded_text):
    pass


# test = 'arroz com feijao'
# print(f'test: {test[0:3]}')

# test_bytes = bytes(test, 'utf-8')
# print(f'test_bytes: {test_bytes}')

# test_hex = test_bytes.hex()
# print(f'test_hex: {test_hex}')

# chico = str(test_bytes)
# print(f'chico: {chico}')

# sys.exit()




# python3 dcc023c2 -c <IP> <port> <input> <output>
# python3 client.py -c 127.0.0.1 2000 input output

# dcc023c2 dcc023c2 0004 faef 00 00 01020304
printDebug('Executando cliente...')

try:

    # Ler Parametros
    ip = str(sys.argv[2])
    port = int(sys.argv[3])
    input = str(sys.argv[4])
    # output = str(sys.argv[5])

    # Abrir arquivos
    inputFD = open(input, "r")
    # outputFD = open(output, "w")

    # Criar cliente
    clientFD = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    clientFD.connect((ip, port))

    # Enviar dados
    raw_chunk = inputFD.read(MSG_LENGTH)
    clientFD.send(encode16(raw_chunk))

except:
    print("Falha:", sys.exc_info())

finally:
    clientFD.close()
    inputFD.close()
    # outputFD.close()

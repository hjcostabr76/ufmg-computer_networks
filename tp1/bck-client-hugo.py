import sys
import socket

# PARÂMETROS (exemplo de como professor vai testar):
# ponta passiva (servidor): ./dcc023c2 -s <port> <input> <output>
# ponta ativa (cliente):    ./dcc023c2 -c <IP> <port> <input> <output>

# Nosso teste é feito assim:
# python client.py -c 127.0.0.1 1236 input output
# python client.py -s 1236 input output
# Precisamos mudar algo nestes parametros acima pra ficar igual do professor ou assim serve?

if __name__ == "__main__":
    
    # python client.py -c 127.0.0.1 1236 input output
    if str(sys.argv[1]) == "-c": # PONTA ATIVA - CLIENTE
        ip = str(sys.argv[2])
        port = int(sys.argv[3])
        input = int(sys.argv[4])
        output = int(sys.argv[5])

        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind((ip, port))
        server.listen(5) # limite de conexoes

        while True:
            client, address = server.accept()
            print(f"Connection Established - {address[0]}:{address[1]}") # essa linha deve ser deletada
            # Dica: Verificar como montar sequências de bytes com dados específicos (p.ex., inteiros de 16 bits) na linguagem escolhida. 
            # No Python, use pacote struct (funções pack e unpack).
            client.close()
            break

    # python client.py -s 1236 input output
    elif (str(sys.argv[1]) == "-s"): # PONTA PASSIVA - SERVIDOR
        ip = "127.0.0.1" # e se não for na mesma máquina ?
        port = int(sys.argv[2])
        input = int(sys.argv[3])
        output = int(sys.argv[4])

        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.connect((ip, port))
        server.close()

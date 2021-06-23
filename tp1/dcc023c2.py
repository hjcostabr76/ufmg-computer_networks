import sys

'''
    TODO: 2021-06-22 - Trazer todo o conteudo dos otros 02 arquivos para este
'''

''' TODO: 2021-06-22 - Implementar servidor aqui '''
def client():
    print('Aqui havera um cliente...')


''' TODO: 2021-06-22 - Implementar servidor aqui '''
def server():
    print('Aqui havera um servidor...')


''' TODO: 2021-06-22 - Implementar servidor aqui '''
def validate_args():
    print('Aqui havera a validacao de entrada...')
    return true


# Validar arquivo
if __name__ != "__main__":
    sys.exit()

# Validar parametros
validate_args()

# Executar
if str(sys.argv[1]) == "-c": # PONTA ATIVA - CLIENTE
    client()

elif (str(sys.argv[1]) == "-s"): # PONTA PASSIVA - SERVIDOR
    server()
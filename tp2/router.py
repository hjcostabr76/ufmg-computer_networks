import sys
from typing import Dict

if __name__ != "__main__":
    sys.exit()


'''
=================================================================
-- Declarar constantes
=================================================================
'''

ENABLE_DEBUG = True

ARG_NAME_ADDR = '--addr'
ARG_NAME_PI = '--update-period'
ARG_NAME_STARTUP = '--startup-commands'

COMMAND_INIT = 'init'
COMMAND_ADD = 'add'
COMMAND_DEL = 'del'
COMMAND_TRACE = 'trace'


'''
=================================================================
-- Declarar funcoes auxiliares ----------------------------------
=================================================================
'''

'''
    Funcao auxiliar para depuracao.
'''
def print_debug(dbtText) -> None:
    if ENABLE_DEBUG:
        print(dbtText)

'''
    TODO: 2021-08-04 - ADD Descricao
'''
def print_instructions_init() -> None:
    print('\nInitialization command formats:')
    print('\n\t- 01: "router.py <IP> <pi>"')
    print('\n\t- 02: "router.py <IP> <pi> <startup file>')
    print('\n\t- 03: "router.py --addr <IP> --update-period <pi>')
    print('\n\t- 04: "router.py --addr <IP> --update-period <pi> --startup-commands <startup file>')

'''
    TODO: 2021-08-04 - ADD Descricao
'''
def print_instructions(command) -> None:

    '''
        TODO: 2021-08-04 - Concluir
    '''

    if (not command):
        print_instructions_init()
    elif (command == COMMAND_INIT):
        print_instructions_init()
    return

'''
Valida & retorna parametros de linha de comando.

Opcoes de comandos de inicializacao:
- 01: python3 router.py <IP> <pi>
- 02: python3 router.py <IP> <pi> <startup file>
- 03: python3 router.py --addr <IP> --update-period <pi>
- 04: python3 router.py --addr <IP> --update-period <pi> --startup-commands <startup file>

'''
def get_cli_params() -> Dict:

    # Detecta formato do comando de incializacao de acordo com a quantidade de argumentos recebidos
    argsc = len(sys.argv)
    command_format = 0
    
    if (argsc <= 5):
        command_format = argsc - 2
    elif (argsc == 7):
        command_format = 4

    if (not command_format in [1, 2, 3, 4]):
        raise IOError('Invalid number of arguments')

    addr = ''
    pi = 0
    startup_path = ''

    # Trata caso de parametros nominais
    
    if (command_format in [3, 4]):

        if (sys.argv[1] != ARG_NAME_ADDR):
            raise IOError('Invalid argument at position 1. (Was it supposed to be "' + ARG_NAME_ADDR + '" ?)')
        if (sys.argv[3] != ARG_NAME_PI):
            raise IOError('Invalid argument at position 3. (Was it supposed to be "' + ARG_NAME_PI + '" ?)')
        if (command_format == 4 and sys.argv[5] != ARG_NAME_STARTUP):
            raise IOError('Invalid argument at position 5. (Was it supposed to be "' + ARG_NAME_STARTUP + '" ?)')

        addr = sys.argv[2]
        pi = sys.argv[4]

        if (command_format == 4):
            startup_path = sys.argv[6]
            if (not startup_path):
                raise IOError('Argument ' + ARG_NAME_STARTUP + ' requires a file path')

    # Trata caso de parametros implicitos
    else:
        pi = sys.argv[2]
        addr = sys.argv[1]
        if (argsc > 3):
            startup_path = sys.argv[3]

    '''
        TODO: 2021-08-04 - Concluir validacao de endereco
    '''

    # Validar endereco
    if (not addr):
        raise IOError('Address is required')
    
    # Validar PI
    if (not pi):
        raise IOError('Update period (pi) is required')

    try:
        pi = float(pi)
    except ValueError:
        raise IOError('Invalid update period (must be a number)')

    '''
        TODO: 2021-08-04 - Validar path do arquivo
    '''

    return {
        'addr': addr,
        'pi': pi,
        'startup_path': startup_path,
    }


'''
=================================================================
-- TODO: ADD algum subtitulo ------------------------------------
=================================================================
'''

print_debug('Running...')

try:

    cli_arguments = get_cli_params()
    print(cli_arguments)

    # # Abrir arquivos
    # inputFD = open(input, "r")
    # # outputFD = open(output, "w")

    # # Criar cliente
    # clientFD = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # clientFD.connect((ip, port))

    # # Enviar dados
    # raw_chunk = inputFD.read(MSG_LENGTH)
    # clientFD.send(encode16(raw_chunk))

except Exception as failure:
    print("\n\n-- FALHA ---------\n", failure)
    print('\n')
    if (ENABLE_DEBUG):
        raise failure

# finally:
#     clientFD.close()
#     inputFD.close()
#     # outputFD.close()

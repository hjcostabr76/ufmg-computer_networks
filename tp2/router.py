import sys
from typing import Dict
import socket

if __name__ != "__main__":
    sys.exit()


'''
=================================================================
-- Declarar constantes
=================================================================
'''

ENABLE_DEBUG = True

PORT = 55151

ARG_NAME_ADDR = '--addr'
ARG_NAME_PI = '--update-period'
ARG_NAME_STARTUP = '--startup-commands'

COMMAND_INIT = 'init'
COMMAND_QUIT = 'quit'
COMMAND_HELP = 'help'

COMMAND_ADD = 'add'
COMMAND_DEL = 'del'
COMMAND_TRACE = 'trace'
COMMAND_ROUTER_LIST = [COMMAND_ADD, COMMAND_DEL, COMMAND_TRACE]


'''
=================================================================
-- Declarar funcoes auxiliares ----------------------------------
=================================================================
'''

'''
    Funcao auxiliar para depuracao.
'''
def print_debug(dbg_text) -> None:
    if ENABLE_DEBUG:
        print(dbg_text)

'''
    Exibe instrucoes de uso de comando: Inicializacao do programa
'''
def print_instructions_init() -> None:
    print('\nInitialization command formats:')
    print('\t- 01: "router.py <IP: string> <pi: float>"')
    print('\t- 02: "router.py <IP: string> <pi: float> <startup_file: string>')
    print('\t- 03: "router.py --addr <IP: string> --update-period <pi: float>')
    print('\t- 04: "router.py --addr <IP: string> --update-period <pi: float> --startup-commands <startup_file: string>')

'''
    Exibe instrucoes de uso de comando: Adicao de roteaodr.
'''
def print_instructions_add() -> None:
    print('\n' + COMMAND_ADD + ' command format:')
    print('\t"router.py ' + COMMAND_ADD + ' <IP: string> <weight: int>"')

'''
    Exibe instrucoes de uso de comando: Remocao de roteaodr.
'''
def print_instructions_del() -> None:
    print('\n' + COMMAND_DEL + ' command format:')
    print('\t"router.py ' + COMMAND_DEL + ' <IP: string>')

'''
    Exibe instrucoes de uso de comando: Rastreamento de roteaodr.
'''
def print_instructions_trace() -> None:
    print('\n' + COMMAND_TRACE + ' command format:')
    print('\t"router.py ' + COMMAND_TRACE + ' <IP: string>')

'''
    Centraliza chamadas para exibicao de instrucoes de uso.
'''
def print_instructions(help_command) -> None:

    print('\nInstructions:')

    if (not help_command or help_command == COMMAND_INIT):
        print_instructions_init()
    if (not help_command or help_command == COMMAND_ADD):
        print_instructions_add()
    if (not help_command or help_command == COMMAND_DEL):
        print_instructions_del()
    if (not help_command or help_command == COMMAND_TRACE):
        print_instructions_trace()

'''
Valida & retorna parametros de linha de comando.

'''
def get_cli_params() -> object:

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

    class return_data: pass
    return_data.addr = addr
    return_data.pi = pi
    return_data.startup_path = startup_path
    return return_data

'''
    Valida linha para comando de exibir instrucoes.
'''
def validate_command_help(command_args) -> None:

    argsc = len(command_args)
    if (argsc > 2):
        raise IOError(COMMAND_HELP + ' command takes exactly 01 or 02 arguments')

    if (argsc == 2 and not command_args[1] in COMMAND_ROUTER_LIST):
        raise IOError('Argument 1 is not a valid router command')

'''
    Avalia & retorna parametros de linha do comando: Add roteador.
'''
def get_command_data_add(command_args) -> object:
    
    argsc = len(command_args)

    try:
        
        if (argsc != 3):
            raise IOError(COMMAND_ADD + ' command takes exactly 03 arguments')

        class return_data: pass
        return_data.router_addr = command_args[1]
        return_data.router_weight = int(command_args[2])
        return return_data

    except IOError as failure:
        print_instructions(COMMAND_ADD)
        raise failure

'''
    Avalia & retorna parametros de linha do comando: Remover roteador.
'''
def get_command_data_del(command_args) -> object:
    
    argsc = len(command_args)

    try:
        
        if (argsc != 2):
            raise IOError(COMMAND_DEL + ' command takes exactly 02 arguments')

        class return_data: pass
        return_data.router_addr = command_args[1]
        return return_data

    except IOError as failure:
        print_instructions(COMMAND_DEL)
        raise failure

'''
    Avalia & retorna parametros de linha do comando: Rastrear roteador.
'''
def get_command_data_trace(command_args) -> object:
    
    argsc = len(command_args)

    try:
        
        if (argsc != 2):
            raise IOError(COMMAND_TRACE + ' command takes exactly 02 arguments')

        class return_data: pass
        return_data.router_addr = command_args[1]
        return return_data

    except IOError as failure:
        print_instructions(COMMAND_TRACE)
        raise failure

'''
    Avalia & retorna parametros de linha de 01 comando generico.
'''
def get_command_data(command_line) -> object:

    command_args = command_line.split()
    command_type = command_args[0]

    if (command_type == COMMAND_ADD):
        parsed_args = get_command_data_add()
    elif (command_type == COMMAND_DEL):
        parsed_args = get_command_data_del()
    elif (command_type == COMMAND_TRACE):
        parsed_args = get_command_data_trace()

    else:

        class parsed_args: pass
        
        if (command_type == COMMAND_HELP):
            validate_command_help(command_args)
            parsed_args.help_command = command_args[1] if len(command_args) == 2 else None

    parsed_args.command = command_type
    return parsed_args

'''
    Executa comando: Exibir instrucoes.
'''
def execute_command_help(command_type) -> None:
    return print_instructions(command_type)

'''
    Executa comando: Add roteador na rede.
'''
def execute_command_add(command_args) -> None:

    argsc = len(command_args)
    if (argsc > 2):
        raise IOError(COMMAND_HELP + ' command takes up to 02 arguments')

    if (argsc == 2):
        command = command_args[1]
        if (not command in COMMAND_ROUTER_LIST):
            raise IOError('Argument 01 is not a valid router command')
    else:
        command = None

    return print_instructions(command)



'''
=================================================================
-- Loop principal -----------------------------------------------
=================================================================
'''

print('\nRunning...')
print('  Type "' + COMMAND_HELP + ' (' + '|'.join([COMMAND_ADD, COMMAND_DEL, COMMAND_TRACE]) + ')?" for instructions;')
print('  Type "' + COMMAND_QUIT + '" to quit;')
# sys.exit()

cli_arguments = None

try:

    cli_arguments = get_cli_params()

    '''
        TODO: 2021-08-04 - ADD Descricao
    '''
    sockFD = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sockFD.connect((cli_arguments.addr, PORT))

    while (True):
        
        # Le comando via CLI
        command_line = input('\nEnter command: ')

        try:
            command_data = get_command_data(command_line)
        except IOError as failure:
            print('Invalid input! >.<\"')
            print(failure)
            continue

        # Executa comando solicitado
        if (command_data.command == COMMAND_QUIT):
            break

        if (command_data.command == COMMAND_HELP):
            execute_command_help(command_data.help_command)

        # # Abrir arquivos
        # inputFD = open(input, "r")
        # # outputFD = open(output, "w")

        

        # # Enviar dados
        # raw_chunk = inputFD.read(MSG_LENGTH)
        # clientFD.send(encode16(raw_chunk))

except Exception as failure:
    
    print("\n\n-- FALHA ---------\n", failure)
    print('\n')

    if (cli_arguments == None):
        print_instructions_init()

    if (ENABLE_DEBUG):
        raise failure

finally:
    sockFD.close()
    print("\n-- THE END --\n")

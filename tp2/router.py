import sys
import json
import time
import socket
import typing
import math

from threading import Thread

if __name__ != "__main__":
    sys.exit()

'''
=================================================================
-- Declarar constantes ------------------------------------------
=================================================================
'''

PORT = 55151
BUF_SIZE = 1024
MAX_PERIODS = 4
INPUT_CLI_MSG = '\nEnter command: '

LOG_LEVEL_DEBUG = 1
LOG_LEVEL_INFO = 2
LOG_LEVEL_HINT = 3
LOG_LEVEL_WARN = 4
LOG_LEVEL_ERROR = 5
LOG_LEVEL = LOG_LEVEL_INFO # Para desbilitar: Setar como 0

MSG_TYPE_DATA = 'data'
MSG_TYPE_UPDATE = 'update'
MSG_TYPE_TRACE = 'trace'

ARG_NAME_ADDR = '--addr'
ARG_NAME_PI = '--update-period'
ARG_NAME_STARTUP = '--startup-commands'

COMMAND_INIT = 'init'
COMMAND_QUIT = 'quit'
COMMAND_HELP = 'help'
COMMAND_DEBUG_TABLE = 'table'

COMMAND_ADD = 'add'
COMMAND_DEL = 'del'
COMMAND_TRACE = 'trace'
COMMAND_ROUTER_LIST = [COMMAND_ADD, COMMAND_DEL, COMMAND_TRACE]


'''
=================================================================
-- Variaveis globais --------------------------------------------
=================================================================
'''

routing_table: dict = {}
main_loop_started = False


'''
=================================================================
-- Declarar funcoes auxiliares ----------------------------------
=================================================================
'''

'''
    Centraliza exibicao de mensagens de log.
'''
def log(level: str, msg) -> None:

    if (level < LOG_LEVEL):
        return

    if (level == LOG_LEVEL_DEBUG):
        level_txt = 'debug'
    elif (level == LOG_LEVEL_INFO):
        level_txt = 'info'
    elif (level == LOG_LEVEL_HINT):
        level_txt = 'hint'
    elif (level == LOG_LEVEL_WARN):
        level_txt = 'warn'
    elif (level == LOG_LEVEL_ERROR):
        level_txt = 'error'
    else:
        return log(LOG_LEVEL_WARN, 'Invalid log type: \'' + level + '\'')

    if (level == LOG_LEVEL_DEBUG):
        print('')
    
    print('[' + level_txt  + ']', msg)

    if (main_loop_started and level != LOG_LEVEL_HINT):
        print(INPUT_CLI_MSG)


'''
    TODO: 2021-08-08 - ADD Descricao
'''
def log_debug(msg) -> None:
    log(LOG_LEVEL_DEBUG, msg)

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def log_hint(msg) -> None:
    log(LOG_LEVEL_HINT, msg)
    pass

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def log_info(msg) -> None:
    log(LOG_LEVEL_INFO, msg)

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def log_warn(msg) -> None:
    log(LOG_LEVEL_WARN, msg)

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def log_error(msg) -> None:
    log(LOG_LEVEL_ERROR, msg)

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def log_messaging(type: str, log: str) -> None:
    if (not type in ['send', 'recv']):
        raise ValueError('Invalid messaging log type ' + type)
    log_debug('[messaging:' + type + '] ' + log)

'''
    Avalia 01 string generica & retorna sua versao de IP caso represente 01 IP valido.
'''
def get_ip_version(addr: str) -> typing.Union[int, bool]:
    try:

        # V4
        try:
            socket.inet_pton(socket.AF_INET, addr)
            return 4

        # V6
        except socket.error:
            socket.inet_pton(socket.AF_INET6, addr)
            return 6

    except socket.error:
        return False

'''
    Valida string quanto a representar um endereco IP valido.
'''
def validate_ip(addr: str, version: int = 4) -> bool:
    if (not version in [4, 6]):
        raise ValueError('Invalid IP version for validation')
    return version == get_ip_version(addr)

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

    # Validar endereco
    if (not addr):
        raise IOError('Address is required')
    if (not validate_ip(addr)):
        raise IOError('Invalid address')
    
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
    Inclui OU atualiza registro de 01 rota na tabela de roteamento.
'''
def set_route(addr_src: str, addr_dst: str, weight: int, is_neighbor: bool) -> None:
    
    destination: dict = routing_table.get(addr_dst)
    if (not destination):
        destination = { 'is_neighbor': is_neighbor }
        routing_table[addr_dst] = destination

    routes: list = destination.get('routes')
    if (not routes):
        routes = []

    exists = False

    for route in routes:
        if (route.get('addr_src') == addr_src):
            exists = True
            route['periods'] = 0
            route['weight'] = weight
            break

    if (not exists):
        routes.append({ 'addr_src': addr_src, 'weight': weight, 'periods': 0 })

    routing_table[addr_dst]['routes'] = routes

'''
    TODO: 2021-08-06 - ADD Descricao
'''
def get_best_route(addr_dest: str) -> typing.Union[dict, None]:

    destination = routing_table.get(addr_dest)
    if (not destination):
        return

    routes = destination.get('routes')
    if (not routes):
        return
    
    best_route: dict = None
    for route in routes:
        if (not best_route or route.get('weight') < best_route.get('weight')):
            best_route = route

    if (best_route):
        return route

'''
    TODO: 2021-08-06 - ADD Descricao
'''
def clear_outdated_routes(addr_dest: str) -> None:
    
    destination = routing_table.get(addr_dest)
    if (not destination):
        return

    routes = destination.get('routes')
    if (not routes or not len(routes)):
        return

    routes_to_pop = []

    for i in range(len(routes)):

        route = routes[i]
        route['periods'] = route.get('periods') + 1

        if (route.get('periods') > MAX_PERIODS):
            routes_to_pop.append(i)
            continue
    
    for i in routes_to_pop:
        routes.pop(i)

'''
    TODO: 2021-08-06 - ADD Descricao
'''
def clear_outdated_destinations() -> None:
    
    addr_dest_list = routing_table.keys()
    addr_to_pop_list = []
    
    for addr in addr_dest_list:
        routes = routing_table.get(addr).get('routes')
        if (not len(routes)):
            addr_to_pop_list.append(addr)
    
    for addr in addr_to_pop_list:
        log_debug('Forgeting route ' + addr + '. We haven''t heard of it for too long :(')
        routing_table.pop(addr)

'''
    Exibe instrucoes de uso de comando: Inicializacao do programa
'''
def print_instructions_init() -> None:
    print('Initialization command formats:')
    print('\t- 01: "router.py <IP: string> <pi: float>"')
    print('\t- 02: "router.py <IP: string> <pi: float> <startup_file: string>')
    print('\t- 03: "router.py --addr <IP: string> --update-period <pi: float>')
    print('\t- 04: "router.py --addr <IP: string> --update-period <pi: float> --startup-commands <startup_file: string>')

'''
    Exibe instrucoes de uso de comando: Adicao de roteaodr.
'''
def print_instructions_add() -> None:
    print(COMMAND_ADD + ' command format:')
    print('\t' + COMMAND_ADD + ' <IP: string> <weight: int>"')

'''
    Exibe instrucoes de uso de comando: Remocao de roteaodr.
'''
def print_instructions_del() -> None:
    print(COMMAND_DEL + ' command format:')
    print('\t' + COMMAND_DEL + ' <IP: string>')

'''
    Exibe instrucoes de uso de comando: Rastreamento de roteaodr.
'''
def print_instructions_trace() -> None:
    print(COMMAND_TRACE + ' command format:')
    print('\t' + COMMAND_TRACE + ' <IP: string>')

'''
    Centraliza chamadas para exibicao de instrucoes de uso.
'''
def print_instructions(help_command: str) -> None:

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
    Valida linha para comando de exibir instrucoes.
'''
def validate_command_help(command_args: list) -> None:

    argsc = len(command_args)
    if (argsc > 2):
        raise IOError(COMMAND_HELP + ' command takes exactly 01 or 02 arguments')

    if (argsc == 2 and not command_args[1] in COMMAND_ROUTER_LIST):
        raise IOError('Argument 1 is not a valid router command')

'''
    Avalia & retorna parametros de linha do comando: Add roteador.
'''
def get_command_data_add(command_args: list) -> object:
    
    argsc = len(command_args)

    if (argsc != 3):
        raise IOError(COMMAND_ADD + ' command takes exactly 03 arguments')

    class return_data: pass

    return_data.addr = command_args[1]
    if (not validate_ip(return_data.addr)):
        raise IOError('Invalid IP address')
    
    return_data.weight = int(command_args[2])
    return return_data

'''
    Avalia & retorna parametros de linha do comando: Remover roteador.
'''
def get_command_data_del(command_args: list) -> object:
    
    if (len(command_args) != 2):
        raise IOError(COMMAND_DEL + ' command takes exactly 02 arguments')

    class return_data: pass
    return_data.addr = command_args[1]
    return return_data

'''
    Avalia & retorna parametros de linha do comando: Rastrear roteador.
'''
def get_command_data_trace(command_args: list) -> object:
    
    if (len(command_args) != 2):
        raise IOError(COMMAND_TRACE + ' command takes exactly 02 arguments')

    class return_data: pass
    return_data.target = command_args[1]
    return return_data

'''
    Avalia & retorna parametros de linha de 01 comando generico.
'''
def get_command_data(command_line: str) -> object:

    command_args = command_line.split()
    command_type = command_args[0]

    try:
        if (command_type == COMMAND_ADD):
            parsed_args = get_command_data_add(command_args)
        elif (command_type == COMMAND_DEL):
            parsed_args = get_command_data_del(command_args)
        elif (command_type == COMMAND_TRACE):
            parsed_args = get_command_data_trace(command_args)

        else:
            class parsed_args: pass
            if (command_type == COMMAND_HELP):
                validate_command_help(command_args)
                parsed_args.help_command = command_args[1] if len(command_args) == 2 else None
            elif (not command_type in [COMMAND_DEBUG_TABLE, COMMAND_INIT, COMMAND_QUIT]):
                raise IOError('Invalid command')

        parsed_args.command = command_type
        return parsed_args
        
    except IOError as error:
        print('\n-- Invalid input! >.<\" --')
        print(error)
        print_instructions(COMMAND_ADD)
        print(INPUT_CLI_MSG)

'''
    Executa comando: Exibir instrucoes.
'''
def execute_command_help(command_type: str) -> None:
    return print_instructions(command_type)

def execute_command_debug_table() -> None:
    print('Routing Table:\n\t', routing_table)
    print(INPUT_CLI_MSG)

'''
    Executa comando: Add roteador na rede.
'''
def execute_command_add(addr_src: str, addr_dst: str, weight: int) -> None:
    set_route(addr_src, addr_dst, weight, True)
    log_info('Address ' + addr_dst + ' successfully added to routing table...')


'''
    Executa comando: Remover roteador na rede.
'''
def execute_command_del(addr: str) -> None:
    if (routing_table.get(addr)):
        routing_table.pop(addr)
        log_info('Address ' + addr + ' successfully removed from routing table...')
    else:
        log_warn('Address ' + addr + ' does not exists in routing table...')

'''
    Executa comando: Rastrear 01 roteador na rede.
'''
def execute_command_trace(src: str, target: str, hops: list = None) -> None:
    for addr_dest in routing_table.keys():
        best_route = get_best_route(addr_dest)
        if (best_route):
            send_msg_trace(src, addr_dest, target, hops if hops != None else [])

'''
    Validador de mensagens: Dados.
'''
def validate_msg_data(msg: dict) -> None:
    if (msg.get('payload') == None):
        raise IOError('Data Message: property "payload" is missing')

'''
    Validador de mensagens: Trace.
'''
def validate_msg_trace(msg: dict) -> None:

    hops: list = msg.get('hops')

    if (hops == None):
        raise IOError('Trace Message: property "hops" is missing')
    if (type(hops) != 'list' or not len(msg.get('hops'))):
        raise IOError('Trace Message: Property "hops" must be a non empty list')

    for i in range(len(hops)):
        if (not validate_ip(hops[i])):
            raise IOError('Trace Message: Invalid IP address in hops list at ' + i + ' position')

'''
    Validador de mensagens: Update.
'''
def validate_msg_update(msg: dict) -> None:

    distances: dict = msg.get('distances')
    if (type(distances) != dict):
        raise IOError('Update Message: Invalid value for distances list (should be of type dict, ' + str(type(distances)) + ' received)')

    for addr_dest in distances.keys():
        
        if (not validate_ip(addr_dest)):
            raise IOError('Update Message: Invalid IP address in distances dict: "' + addr_dest + '"')

        weight = distances.get(addr_dest)
        if (type(weight) != int or weight <= 0): 
            raise IOError('Update Message: Weight for address ' + addr_dest + ' should be a positive int ("' + weight + '" provided)')

'''
    Encapsula procedimento generico de validacao de mensagens.
'''
def validate_msg(msg: dict) -> None:

    # Valida presenca de campos obrigatorios
    msg_type: str = msg.get('type')
    
    if (msg_type == None):
        raise IOError('All Messages must have the "type" property')
    if (msg.get('source') == None):
        raise IOError('All Messages must have the "source" property')
    if (msg.get('destination') == None):
        raise IOError('All Messages must have the "destination" property')

    # Valida valores dos campos obrigatorios    
    if (not msg_type in [MSG_TYPE_DATA, MSG_TYPE_TRACE, MSG_TYPE_UPDATE]):
        raise IOError('Invalid message type "' + msg_type + '"')
    if (not validate_ip(msg.get('source'))):
        raise IOError('Invalid IP address received as "source"')
    if (not validate_ip(msg.get('destination'))):
        raise IOError('Invalid IP address received as "destination"')

    # Valida campos especificos de cada tipo de msg
    if (msg_type == MSG_TYPE_DATA):
        validate_msg_data(msg)
    elif (msg_type == MSG_TYPE_TRACE):
        validate_msg_trace(msg)
    elif (msg_type == MSG_TYPE_UPDATE):
        validate_msg_update(msg)

'''
    Encapsula procedimento de envio de quaisquer mensagens.
'''
def send_msg(src: str, dest: str, msg: dict) -> None:
    try:
        addr_family = socket.AF_INET if get_ip_version(src) == 4 else socket.AF_INET6
        senderFD = socket.socket(addr_family, socket.SOCK_DGRAM)
        senderFD.sendto(json.dumps(msg).encode(), (dest, PORT))

    except socket.error as error:
        log_error('Failure as sending ' + msg.get('type') + ' message')
        if (LOG_LEVEL <= LOG_LEVEL_DEBUG):
            raise error

    finally:
        senderFD.close()

'''
    Encapsula procedimento de envio de mensagens: Dados.
'''
def send_msg_data(src: str, dest: str, payload) -> None:

    msg = {
        'type': MSG_TYPE_DATA,
        'source': src,
        'destination': dest,
        'payload': payload,
    }

    send_msg(src, dest, msg)

'''
    Encapsula procedimento de envio de mensagens: Trace.
'''
def send_msg_trace(src: str, dest: str, target: str, hops: list) -> None:

    msg = {
        'type': MSG_TYPE_TRACE,
        'source': src,
        'destination': target,
        'hops': hops,
    }

    send_msg(src, dest, msg)

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def send_msg_update(addr_src: str, addr_dest: str, weight: int) -> None:

    distances: dict = {}
    distances[addr_src] = weight

    for addr in routing_table.keys():
        
        if (addr == addr_dest):
            continue

        for route in routing_table.get(addr).get('routes'):
            if (route.get('addr_src') != addr_dest):
                distances[addr] = weight + route.get('weight')

    msg = {
        'type': MSG_TYPE_UPDATE,
        'source': addr_src,
        'destination': addr_dest,
        'distances': distances,
    }

    log_messaging('send', 'Sending ' + MSG_TYPE_UPDATE + ' message to: ' + addr_dest)
    send_msg(addr_src, addr_dest, msg)

'''
    Handler para avaliacao de mensgens: Dados.
'''
def handle_msg_data(src: str, payload) -> None:
    log_debug('\t' + src + ' says: ', payload)

'''
    Handler para avaliacao de mensgens: Trace.
'''
def handle_msg_trace(src: str, msg: dict) -> None:
    
    # Propaga msg de rastreamento (quando alvo for outro roteador)
    hops = msg.get('hops') + [src]
    if (msg.get('destination') != src):
        return execute_command_trace(src, msg.get('destination'), hops)
    
    # Responde msg de rastreamento (quando alvo for este roteador)
    msg['hops'] = hops
    send_msg_data(src, msg.get('source'), msg)

'''
    TODO: 2021-08-08 - ADD Descricao
'''
def handle_msg_update(addr_src: str, distances: dict) -> None:
    for addr_dest, weight in distances.items():
        set_route(addr_src, addr_dest, weight, addr_src == addr_dest)

'''
    Handler generico para avaliacao de mensgens recebidas.
'''
def handle_msg(raw_msg: bytes) -> None:
    try:

        msg: dict = json.loads(raw_msg)
        addr_src = msg.get('source') if msg.get('source') else '?'
        validate_msg(msg)
        
        msg_type = msg.get('type')
        log_messaging('recv', 'New message received from: ' + addr_src)

        if (msg_type == MSG_TYPE_UPDATE):
            handle_msg_update(msg.get('source'), msg.get('distances'))
        elif (msg_type == MSG_TYPE_TRACE):
            handle_msg_trace(addr_src, msg)
        elif (msg_type == MSG_TYPE_DATA):
            handle_msg_data(addr_src, msg)

    except IOError as error:
        log_error('Falha ao receber mensagem de: ' + addr_src)
        log_error(error)
        log_debug(msg)

'''
    Thread para atualizacao periodica da tabela de roteamento:
    - Atualiza a idade de cada rota conhecida;
    - Remove da tabela rotas desatualizadas;
    - Envia mensagens de update para atualizacao de rotas dos vizinhos;
'''
def thread_update_table(addr: str, pi: float) -> None:

    log_info('Ready to send update messages from: ' + addr + ':' + str(PORT) + '...')
    
    idled_periods = 0
    idling_checkpoint = 8

    while True:
        
        time.sleep(pi)
        addr_dest_list = routing_table.keys()

        # Notifica em caso de multiplos periodos sem atualizacao        
        if (not len(addr_dest_list)):
            idled_periods = idled_periods + 1
            if (idled_periods > idling_checkpoint):
                idled_periods = 0
                log_debug('No neighbors to send messages...')
            continue

        # Atualiza rotas / destinos da tabela de roteamento            
        for addr_dest in addr_dest_list:
            
            clear_outdated_routes(addr_dest)
            
            if (not routing_table.get(addr_dest).get('is_neighbor')):
                continue

            best_route = get_best_route(addr_dest)
            if (best_route):
                send_msg_update(addr, addr_dest, best_route.get('weight'))

        clear_outdated_destinations()

'''
    Thread para recebimento de mensagens de roteadores vizinhos.
'''
def thread_listen_msgs(addr: str) -> None:
    try:
        addr_family = socket.AF_INET if get_ip_version(addr) == 4 else socket.AF_INET6
        listenerFD = socket.socket(addr_family, socket.SOCK_DGRAM)
        listenerFD.bind((addr, PORT))

        log_info('Listening for update messages at: ' + addr + ':' + str(PORT) + '...')

        while True:
            raw_msg = listenerFD.recv(BUF_SIZE)
            handle_msg(raw_msg)

    except socket.error as error:
        log_error('Update message listener failed to start')
        if (LOG_LEVEL <= LOG_LEVEL_DEBUG):
            raise error

    finally:
        listenerFD.close()


'''
=================================================================
-- Loop principal -----------------------------------------------
=================================================================
'''

print('\nRunning...\n')
log_hint('Type "' + COMMAND_HELP + ' (' + '|'.join([COMMAND_ADD, COMMAND_DEL, COMMAND_TRACE]) + ')?" for instructions;')
log_hint('Type "' + COMMAND_QUIT + '" to quit;')

cli_arguments = None

try:

    cli_arguments = get_cli_params()

    # Thread: Acoes de atualizacao da tabela de roteamento
    update_sender = Thread(target=thread_update_table, args=(cli_arguments.addr, cli_arguments.pi))
    update_sender.start()

    # Thread: ESCUTAR msgs de update
    update_listener = Thread(target=thread_listen_msgs, args=(cli_arguments.addr,))
    update_listener.start()

    time.sleep(1)
    main_loop_started = True
    print(INPUT_CLI_MSG)

    while (True):
        
        # Le comando via CLI
        command_line = input()
        command_data = get_command_data(command_line)
        
        if (not command_data):
            continue

        # Executa comando solicitado
        if (command_data.command == COMMAND_QUIT):
            break

        if (command_data.command == COMMAND_HELP):
            execute_command_help(command_data.help_command)
        elif (command_data.command == COMMAND_DEBUG_TABLE):
            execute_command_debug_table()
        elif (command_data.command == COMMAND_ADD):
            execute_command_add(cli_arguments.addr, command_data.addr, command_data.weight)
        elif (command_data.command == COMMAND_DEL):
            execute_command_del(command_data.addr)
        elif (command_data.command == COMMAND_TRACE):
            execute_command_trace(command_data.addr, command_data.target)

except Exception as error:
    
    log_error("\n--------- FAILURE ---------")
    log_error(error)
    
    if (cli_arguments == None):
        print_instructions_init()
    if (LOG_LEVEL <= LOG_LEVEL_DEBUG):
        raise error

finally:
    log_info("\n-- THE END --\n")

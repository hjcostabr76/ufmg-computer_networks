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
LOG_LEVEL = LOG_LEVEL_DEBUG # Para desbilitar: Setar como 0

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
address: str = ''
have_main_loop_started = False
should_stop_threads = False


'''
=================================================================
-- Declarar funcoes genericas / auxiliares / utilitarias --------
=================================================================
'''

'''
    Avalia & infomra se 01 nivel de emissao de log esta habilitado.
'''
def is_log_level_valid(level: int) -> bool:
    return (
        level in [LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_HINT, LOG_LEVEL_WARN, LOG_LEVEL_ERROR]
        and level>= LOG_LEVEL
    )

'''
    Centraliza exibicao de mensagens de log.
'''
def log(level: str, msg) -> None:

    if (not is_log_level_valid(level)):
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

    if (level == LOG_LEVEL_DEBUG):
        print('')
    
    print('[' + level_txt  + ']', msg)

    if (have_main_loop_started and not level in [LOG_LEVEL_HINT, LOG_LEVEL_ERROR]):
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
=================================================================
-- Declarar funcoes do dominio do problema ----------------------
=================================================================
'''

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
def set_route(addr_src: str, addr_dest: str, weight: int, is_neighbor: bool) -> None:
    
    destination: dict = routing_table.get(addr_dest)
    if (not destination):
        destination = { 'is_neighbor': is_neighbor }
        routing_table[addr_dest] = destination

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

    if (is_neighbor):
        routing_table[addr_dest]['is_neighbor'] = True
    routing_table[addr_dest]['routes'] = routes

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
    Identifica & rota para 01 determinado destino atraves do vizinho que informou o melhor caminho.
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
    Remove rotas desatualizadas para 01 determinado destino:
    - Atualiza contagem de tempo sem atualizacao das rotas de 01 destino;
    - Remove rotas informadas por algum vizinho que nao tenham sido atualizadas por mais tempo que o limite;
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
        is_self_mapped_neighbor = destination.get('is_neighbor') and route.get('addr_src') == address
        if (is_self_mapped_neighbor):
            continue

        route['periods'] = route.get('periods') + 1
        if (route.get('periods') > MAX_PERIODS):
            routes_to_pop.append(i)
    
    for i in routes_to_pop:
        routes.pop(i)

'''
    Remove da tabela de roteamento destinos para os quais nao restam nenhuma rota.
'''
def clear_outdated_destinations() -> None:
    
    addr_to_pop_list = []
    
    for addr_dest in routing_table.keys():
        routes = routing_table.get(addr_dest).get('routes')
        if (not len(routes)):
            addr_to_pop_list.append(addr_dest)
    
    for addr_dest in addr_to_pop_list:
        log_debug('Forgeting route ' + addr_dest + '. We haven''t heard of it for too long :(')
        routing_table.pop(addr_dest)

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

    destination = routing_table.get(addr)
    if (not destination):
        return log_warn('Address ' + addr + ' does not exist in routing table...')

    if (not destination.get('is_neighbor')):
        return log_warn('Address ' + addr + ' is not a neighbor one...')

    routing_table.pop(addr)
    log_info('Address ' + addr + ' successfully removed from routing table...')

'''
    Executa comando: Rastrear 01 roteador na rede.
'''
def execute_command_trace(addr_src: str, addr_target: str, hops: list = None) -> None:
    
    has_neighbors = False
    sent = False

    for addr_dest in routing_table.keys():

        if (not routing_table.get(addr_dest).get('is_neighbor')):
            continue
        has_neighbors = True

        if (addr_dest != addr_src):
            send_msg_trace(addr_src, addr_target, hops if hops != None else [address])
            sent = True

    # Notifica em caso de envio nao ocorrer por alguma razao
    if (not has_neighbors):
        return log_warn('No neighbors to send trace request')
    if (not sent):
        return log_warn('Somehing wrong isn''t right! No messages sent for this trace request :(')
    
    # Notifica envio quando estamos iniciando rastreamento agora
    if (addr_src == address):
        log_info('Trace request successfully sent')

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
    if (type(hops) != list or not len(msg.get('hops'))):
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
def send_msg(msg: dict) -> None:
    try:
        
        senderFD: socket.socket = None

        # Define vizinho para o qual essa msg sera enviada
        addr_target = msg.get('destination')
        best_route = get_best_route(addr_target)
        if (not best_route):
            raise RuntimeError('No route known for destination ' + addr_target)

        addr_dest = best_route.get('addr_src')
        if (addr_dest == address):
            addr_dest = addr_target

        # Verifica se destino do envio eh nosso vizinho
        if (not routing_table.get(addr_dest).get('is_neighbor')):
            return log_warn('Message to ' + addr_dest + ' won\'t be sent as it is not a neighbor... :(')

        # Envia msg para vizinho que possui a melhor rota para o destino solicitado
        addr_family = socket.AF_INET if get_ip_version(address) == 4 else socket.AF_INET6
        senderFD = socket.socket(addr_family, socket.SOCK_DGRAM)
        senderFD.sendto(json.dumps(msg).encode(), (addr_dest, PORT))

    except socket.error as error:
        log_error('Failure as sending ' + msg.get('type') + ' message')
        if (is_log_level_valid(LOG_LEVEL_DEBUG)):
            raise error

    except Exception as error:
        log_error(error)

    finally:
        if (senderFD):
            senderFD.close()

'''
    Encapsula procedimento de envio de mensagens: Dados.
'''
def send_msg_data(addr_src: str, add_dest: str, payload) -> None:
    send_msg({
        'type': MSG_TYPE_DATA,
        'source': addr_src,
        'destination': add_dest,
        'payload': payload,
    })

'''
    Encapsula procedimento de envio de mensagens: Trace.
'''
def send_msg_trace(addr_src: str, addr_dest: str, hops: list) -> None:
    send_msg({
        'type': MSG_TYPE_TRACE,
        'source': addr_src,
        'destination': addr_dest,
        'hops': hops,
    })

'''
    Encapsula procedimento de envio de mensagens: Update:
    - Inclui peso para chegar a mim;
    - Nao falo pro destino como chegar nele mesmo;
    - O menor peso para 01 dos meus vizinhos eh o peso para chegar a mim + o menor peso para eu chegar nesse vizinho;
'''
def send_msg_update(addr_dest: str) -> None:

    distances: dict = {}
    warn_msg_no_best_route = '[update: send] Something wrong isn''t right! No best route found for neighbor ' + addr_dest

    # Inclui peso para chegar a mim
    best_route_dest = get_best_route(addr_dest)
    if (not best_route_dest):
        return log_warn(warn_msg_no_best_route)
    
    distances[address] = best_route_dest.get('weight')

    # Inclui menor peso chegar em cada 01 dos meus vizinhos
    for addr in routing_table.keys():

        if (addr == addr_dest):
            continue

        best_route = get_best_route(addr)
        if (not best_route):
            log_warn(warn_msg_no_best_route)
            continue

        if (best_route.get('addr_src') != addr_dest):
            distances[addr] = best_route_dest.get('weight') + best_route.get('weight')

    send_msg({
        'type': MSG_TYPE_UPDATE,
        'source': address,
        'destination': addr_dest,
        'distances': distances,
    })

'''
    Handler para avaliacao de mensgens: Dados.
'''
def handle_msg_data(msg: dict) -> None:
    if (msg.get('destination') != address):
        send_msg_data(msg.get('source'), msg.get('destination'), msg.get('payload'))
    else:
        if (is_log_level_valid(LOG_LEVEL_INFO)):
            print('\t' + msg.get('source') + ' says: ', msg.get('payload'))

'''
    Handler para avaliacao de mensgens: Trace.
'''
def handle_msg_trace(msg: dict) -> None:
    
    # Propaga msg de rastreamento (quando alvo for outro roteador)
    hops = msg.get('hops') + [address]
    if (msg.get('destination') != address):
        return execute_command_trace(msg.get('source'), msg.get('destination'), hops)
    
    # Responde msg de rastreamento (quando alvo for este roteador)
    msg['hops'] = hops
    send_msg_data(address, msg.get('source'), msg)

'''
    Handler para avaliacao de mensgens: Update.
'''
def handle_msg_update(msg: dict) -> None:
    for addr_dest, weight in msg.get('distances').items():
        set_route(msg.get('source'), addr_dest, weight, False)

'''
    Handler generico para avaliacao de mensgens recebidas.
'''
def handle_msg(raw_msg: bytes) -> None:
    try:

        msg: dict = json.loads(raw_msg)
        validate_msg(msg)
        
        msg_type = msg.get('type')

        if (msg_type == MSG_TYPE_UPDATE):
            handle_msg_update(msg)
        elif (msg_type == MSG_TYPE_TRACE):
            handle_msg_trace(msg)
        elif (msg_type == MSG_TYPE_DATA):
            handle_msg_data(msg)

    except IOError as error:
        log_warn('Falha ao receber mensagem de: ' + msg.get('source') if msg.get('source') else '?')
        log_warn(error)
        log_debug(msg)

    except Exception as error:
        log_error(error)
        log_debug(raw_msg)

'''
    Thread para atualizacao periodica da tabela de roteamento:
    - Atualiza a idade de cada rota conhecida;
    - Remove da tabela rotas desatualizadas;
    - Envia mensagens de update para atualizacao de rotas dos vizinhos;
'''
def thread_update_table(pi: float) -> None:

    log_info('Ready to send update messages from: ' + address + ':' + str(PORT) + '...')
    
    while not should_stop_threads:
        time.sleep(pi)
        
        for addr_dest in routing_table.keys():
            clear_outdated_routes(addr_dest)
            if (routing_table.get(addr_dest).get('is_neighbor')):
                send_msg_update(addr_dest)

        clear_outdated_destinations()

'''
    Thread para recebimento de mensagens de roteadores vizinhos.
'''
def thread_listen_msgs() -> None:
    try:
        addr_family = socket.AF_INET if get_ip_version(address) == 4 else socket.AF_INET6
        listenerFD = socket.socket(addr_family, socket.SOCK_DGRAM)
        listenerFD.bind((address, PORT))

        log_info('Listening for update messages at: ' + address + ':' + str(PORT) + '...')

        while not should_stop_threads:
            raw_msg = listenerFD.recv(BUF_SIZE)
            handle_msg(raw_msg)

    except socket.error as error:
        log_error('Update message listener failed to start')
        if (is_log_level_valid(LOG_LEVEL_DEBUG)):
            raise error

    finally:
        listenerFD.close()

'''
    Executa encerramento das threads abertas
'''
def threads_finish_em_all() -> bool:
    should_stop_threads = True
    while update_sender.is_alive() or update_listener.is_alive():
        time.sleep(.001)
    return True

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
    address = cli_arguments.addr

    # Thread: Acoes de atualizacao da tabela de roteamento
    update_sender = Thread(target=thread_update_table, args=(cli_arguments.pi,))
    update_sender.start()

    # Thread: ESCUTAR msgs de update
    update_listener = Thread(target=thread_listen_msgs)
    update_listener.start()

    time.sleep(1)
    have_main_loop_started = True
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
            execute_command_add(address, command_data.addr, command_data.weight)
        elif (command_data.command == COMMAND_DEL):
            execute_command_del(command_data.addr)
        elif (command_data.command == COMMAND_TRACE):
            execute_command_trace(address, command_data.target)

    log_info("\n-- THE END --\n")

except Exception as error:
    
    log_error("\n--------- FAILURE ---------")
    log_error(error)
    
    if (cli_arguments == None):
        print_instructions_init()
    if (is_log_level_valid(LOG_LEVEL_DEBUG)):
        raise error

finally:
    threads_finish_em_all()
    sys.exit()

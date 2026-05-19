import os
import socket
import threading
import time

# =========================
# Configuration
# =========================
HOST = "127.0.0.1"
PORT = 5000

AUCTION_DURATION = int(os.environ.get("AUCTION_DURATION", 20))
MIN_INCREMENT = 50
EXPECTED_CLIENTS = int(os.environ.get("EXPECTED_CLIENTS", 3))

# =========================
# Log
# =========================
LOG_FILE = None


def open_log_file(filename):
    global LOG_FILE
    LOG_FILE = open(filename, "w", encoding="utf-8")


def close_log_file():
    global LOG_FILE
    if LOG_FILE is not None:
        LOG_FILE.close()
        LOG_FILE = None


def log_message(message):
    print(message, flush=True)
    if LOG_FILE is not None:
        LOG_FILE.write(message + "\n")
        LOG_FILE.flush()


# =========================
# Global State
# =========================

items = [
    {"name": "Laptop", "base_price": 500},
    {"name": "Phone", "base_price": 300},
    {"name": "Tablet", "base_price": 400},
]

clients = []
client_names = {}
client_files = {}
client_active = {}
passed_current_item = {}

clients_lock = threading.Lock()
auction_lock = threading.Lock()
bid_event = threading.Event()
stop_event = threading.Event()

server_socket = None
accept_thread = None
auction_thread = None
client_threads = []
accepting_clients = True
auction_started = False
current_item_index = -1
current_price = 0
current_winner = None
current_winner_name = None
auction_active = False
auction_end_time = 0.0


# =========================
# Utilities
# =========================
def safe_shutdown_close(sock):
    try:
        sock.shutdown(socket.SHUT_RDWR)
    except:
        pass
    try:
        sock.close()
    except:
        pass


def send_message(sock, message):
    try:
        sock.sendall((message + "\n").encode("utf-8"))
        return True
    except:
        return False


def broadcast(message):
    with clients_lock:
        snapshot = list(clients)

    for sock in snapshot:
        if client_active.get(sock, False):
            if not send_message(sock, message):
                remove_client(sock)


def remove_client(sock):
    file_obj = None
    with clients_lock:
        client_active[sock] = False
        if sock in clients:
            clients.remove(sock)
        client_names.pop(sock, None)
        file_obj = client_files.pop(sock, None)
        passed_current_item.pop(sock, None)

    if file_obj is not None:
        try:
            file_obj.close()
        except:
            pass

    safe_shutdown_close(sock)


def close_all_clients():
    with clients_lock:
        snapshot = list(clients)
    for sock in snapshot:
        remove_client(sock)


def get_current_item():
    if 0 <= current_item_index < len(items):
        return items[current_item_index]
    return None


def reset_pass_flags():
    with clients_lock:
        for sock in clients:
            passed_current_item[sock] = False


# =========================
# Command Processing
# =========================
def process_view(sock):
    with auction_lock:
        item = get_current_item()
        if item is None:
            send_message(sock, "[SERVER] VIEW NO_MORE_ITEMS")
        elif auction_active:
            send_message(sock, f"[SERVER] VIEW ITEM={item['name']} PRICE={current_price} LEADER={current_winner_name}")
        else:
            send_message(sock, "[SERVER] VIEW NO_ACTIVE_AUCTION")


def process_pass(sock):
    with clients_lock:
        passed_current_item[sock] = True
        name = client_names.get(sock, "Unknown")
    send_message(sock, "[SERVER] OK PASS")
    broadcast(f"[SERVER] PASS NAME={name}")


def process_bid(sock, parts):
    global current_price, current_winner, current_winner_name, auction_end_time

    if len(parts) != 2:
        send_message(sock, "[SERVER] ERROR INVALID_BID_FORMAT")
        return

    try:
        amount = int(parts[1])
    except ValueError:
        send_message(sock, "[SERVER] ERROR INVALID_BID_FORMAT")
        return

    with clients_lock:
        name = client_names.get(sock, "Unknown")

    with auction_lock:
        if not auction_active:
            send_message(sock, "[SERVER] ERROR NO_ACTIVE_AUCTION")
            return

        min_valid = current_price + MIN_INCREMENT
        if amount < min_valid:
            send_message(sock, f"[SERVER] ERROR BID_TOO_LOW MIN={min_valid}")
            return

        current_price = amount
        current_winner = sock
        current_winner_name = name
        auction_end_time = time.time() + AUCTION_DURATION

    with clients_lock:
        passed_current_item[sock] = False

    send_message(sock, f"[SERVER] OK BID_ACCEPTED PRICE={amount}")
    broadcast(f"[SERVER] NEW_BID NAME={name} PRICE={amount}")
    bid_event.set()


def process_exit(sock):
    send_message(sock, "[SERVER] OK EXIT")
    remove_client(sock)


# =========================
# Threads
# =========================
def handle_client(sock, addr):
    try:
        file_obj = sock.makefile("r", encoding="utf-8")

        send_message(sock, "[SERVER] ENTER_NAME")

        name = file_obj.readline()

        if not name:
            remove_client(sock)
            return

        name = name.strip()

        if name == "":
            remove_client(sock)
            return

        with clients_lock:
            client_names[sock] = name
            client_files[sock] = file_obj
            client_active[sock] = True
            passed_current_item[sock] = False

        send_message(sock, f"[SERVER] HELLO NAME={name}")
        log_message(f"[SERVER] CLIENT_REGISTERED NAME={name} ADDR={addr}")

        while not stop_event.is_set():
            line = file_obj.readline()

            if not line:
                break

            message = line.strip()

            if message == "":
                continue

            parts = message.split()
            command = parts[0].upper()

            if command == "VIEW":
                process_view(sock)
            elif command == "BID":
                process_bid(sock, parts)
            elif command == "PASS":
                process_pass(sock)
            elif command == "EXIT":
                process_exit(sock)
                return
            else:
                send_message(sock, "[SERVER] ERROR INVALID_COMMAND")

    except:
        pass

    remove_client(sock)


def accept_clients_loop():
    global accepting_clients

    log_message(f"[SERVER] LISTENING HOST={HOST} PORT={PORT}")

    while accepting_clients:
        try:
            sock, addr = server_socket.accept()
        except:
            break

        if auction_started:
            send_message(sock, "[SERVER] ERROR AUCTION_ALREADY_STARTED")
            safe_shutdown_close(sock)
            continue

        with clients_lock:
            clients.append(sock)
            client_active[sock] = True
            count = len(clients)

        t = threading.Thread(target=handle_client, args=(sock, addr))
        t.start()
        client_threads.append(t)

        if count >= EXPECTED_CLIENTS:
            accepting_clients = False


def auction_loop():
    global auction_started, current_item_index, current_price, current_winner
    global current_winner_name, auction_active, auction_end_time

    auction_started = True

    for i, item in enumerate(items):
        with auction_lock:
            current_item_index = i
            current_price = item["base_price"]
            current_winner = None
            current_winner_name = None
            auction_active = True
            auction_end_time = time.time() + AUCTION_DURATION

        reset_pass_flags()
        bid_event.clear()
        msg_start = f"[SERVER] AUCTION_START ITEM={item['name']} BASE={item['base_price']} DURATION={AUCTION_DURATION}"
        log_message(msg_start)
        broadcast(msg_start)

        while True:
            with auction_lock:
                remaining = int(auction_end_time - time.time())

            if remaining <= 0:
                break

            msg_time = f"[SERVER] TIME_LEFT ITEM={item['name']} SECONDS={remaining}"
            log_message(msg_time)
            broadcast(msg_time)
            bid_event.wait(timeout=1.0)
            bid_event.clear()

        with auction_lock:
            auction_active = False
            winner_name = current_winner_name
            final_price = current_price

        if winner_name:
            msg_end = f"[SERVER] AUCTION_END ITEM={item['name']} WINNER={winner_name} PRICE={final_price}"
        else:
            msg_end = f"[SERVER] AUCTION_END ITEM={item['name']} WINNER=None"
        log_message(msg_end)
        broadcast(msg_end)

    broadcast("[SERVER] SERVER_SHUTDOWN")
    stop_event.set()


# =========================
# Start / Shutdown
# =========================
def start_server():
    global server_socket, accept_thread, auction_thread

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen()

    accept_thread = threading.Thread(target=accept_clients_loop)
    accept_thread.start()

    while True:
        with clients_lock:
            count = len(clients)
        if count >= EXPECTED_CLIENTS:
            break
        time.sleep(0.1)

    auction_thread = threading.Thread(target=auction_loop)
    auction_thread.start()

    auction_thread.join()

    safe_shutdown_close(server_socket)
    accept_thread.join()

    close_all_clients()

    for t in client_threads:
        t.join()

    log_message("[SERVER] SERVER_CLOSED")
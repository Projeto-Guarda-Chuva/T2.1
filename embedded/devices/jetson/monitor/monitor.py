import socket
import os
import threading
import requests

SOCKET_PATH = "/tmp/monitor.sock"
url = ""  # perguntar qual a porta

def handle_connection(conn):
    buffer = conn.makefile('r', enconding='utf-8')

    try:
        while True:
            line = buffer.readline()
            json_message = line.strip()
            # print(f"JSON recebido: {json_message}")
            response = requests.post(url)
            print(f"Resposta da CM: {response.status_code}")

    except Exception as e:
        print(e)
    
    finally:
        buffer.close()
        conn.close()

def start_jetson_monitor():
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)

    server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    server.bind(SOCKET_PATH)

    server.listen(5) # número de conexões simultâneas no Kernle 
    print("Monitor Jetson iniciado.")

    try:
        while True:
            conn, _ = server.accept()
            thread = threading.Thread(target=handle_connection, args=(conn))

            thread.daemon = True
            thread.start()
    
    except Exception as e:
        print(e)

    finally:
        server.close()

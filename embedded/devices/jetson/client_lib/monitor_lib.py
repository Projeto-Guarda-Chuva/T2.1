import json
from datetime import datetime, timezone
import socket
import threading

SOCKET_PATH = "/tmp/monitor.sock"

def _create_timestamp():
    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

def _create_json_message(component: str, data: dict) -> str:
    message = {
        "timestamp": _create_timestamp(),
        "device": "NVIDIA_Jetson",
        "component": component, 
        "type": "event", # "event" para dados de um componente da jetson/task das ESP, "telemetry" para dados de monitoramento de rede e hardware 
        "data": data # dados efetivamente a sereme enviados, formato JSON
    }

    return json.dumps(message) + "\n"

def _send_worker(message: str):
    client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    try:
        client.connect(SOCKET_PATH)
        client.sendall(message.encode('utf-8'))
        print("Mensagem enviada com sucesso para o Monitor Jetson.")

    except Exception as e:
        print(e)
    
    finally:
        client.close()    

def send_to_monitor(component: str, data: dict):
    message = _create_json_message(component, data)
    thread = threading.Thread(target=_send_worker, args= (message))
    thread.start()
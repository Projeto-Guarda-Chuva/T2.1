import contextlib
import io
import json
import os
import shutil
import socket
import tempfile
import threading
import unittest
from unittest import mock

import _paths  
import jetson_monitor
import monitor_lib


class ClientToServerSocketTests(unittest.TestCase):
    def setUp(self) -> None:
        self._tmpdir = tempfile.mkdtemp(prefix="t21_sock_")
        self.sock_path = os.path.join(self._tmpdir, "monitor.sock")

        self.server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.server.bind(self.sock_path)
        self.server.listen(1)
        self.server.settimeout(5)

        self._received: list[bytes] = []
        self._accept_thread = threading.Thread(target=self._accept_once)
        self._accept_thread.start()

    def tearDown(self) -> None:
        self._accept_thread.join(timeout=5)
        self.server.close()
        shutil.rmtree(self._tmpdir, ignore_errors=True)

    def _accept_once(self) -> None:
        conn, _ = self.server.accept()
        try:
            chunks = []
            while True:
                data = conn.recv(4096)
                if not data:
                    break
                chunks.append(data)
            self._received.append(b"".join(chunks))
        finally:
            conn.close()

    def test_message_travels_end_to_end_and_is_parseable(self) -> None:
        message = monitor_lib._create_json_message("camera", {"frames": 42})

        with mock.patch.object(monitor_lib, "SOCKET_PATH", self.sock_path):
            monitor_lib._send_worker(message)

        self._accept_thread.join(timeout=5)
        self.assertEqual(len(self._received), 1)

        payload = json.loads(self._received[0].decode("utf-8").strip())
        self.assertEqual(payload["device"], "NVIDIA_Jetson")
        self.assertEqual(payload["component"], "camera")
        self.assertEqual(payload["data"], {"frames": 42})


class HandleConnectionTests(unittest.TestCase):
    def test_handle_connection_reads_and_prints_received_json(self) -> None:
        server_end, client_end = socket.socketpair(socket.AF_UNIX, socket.SOCK_STREAM)

        captured = io.StringIO()

        def run_handler() -> None:
            with contextlib.redirect_stdout(captured):
                jetson_monitor.handle_connection(server_end)

        handler_thread = threading.Thread(target=run_handler)
        handler_thread.start()

        message = monitor_lib._create_json_message("motor", {"state": "OPENING"})
        client_end.sendall(message.encode("utf-8"))
        client_end.close()

        handler_thread.join(timeout=5)
        self.assertFalse(handler_thread.is_alive())

        output = captured.getvalue()
        self.assertIn("JSON recebido:", output)
        self.assertIn('"component": "motor"', output)


if __name__ == "__main__":
    unittest.main()

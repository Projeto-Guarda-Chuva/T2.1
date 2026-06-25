import json
import unittest
from unittest import mock

import _paths
import monitor_lib


class CreateJsonMessageTests(unittest.TestCase):
    def test_message_contains_all_required_fields(self) -> None:
        raw = monitor_lib._create_json_message("camera", {"frames": 30})
        payload = json.loads(raw)

        for field in ("timestamp", "device", "component", "type", "data"):
            self.assertIn(field, payload)

    def test_message_has_fixed_contract_values(self) -> None:
        payload = json.loads(monitor_lib._create_json_message("motor", {}))

        self.assertEqual(payload["device"], "NVIDIA_Jetson")
        self.assertEqual(payload["type"], "event")

    def test_message_forwards_component_and_data(self) -> None:
        data = {"rssi": -42, "ok": True}
        payload = json.loads(monitor_lib._create_json_message("rede", data))

        self.assertEqual(payload["component"], "rede")
        self.assertEqual(payload["data"], data)

    def test_message_is_newline_delimited(self) -> None:
        raw = monitor_lib._create_json_message("camera", {"frames": 1})

        self.assertTrue(raw.endswith("\n"))
        self.assertEqual(raw.count("\n"), 1)

    def test_message_body_is_valid_json(self) -> None:
        raw = monitor_lib._create_json_message("camera", {"frames": 1})

        json.loads(raw.strip())


class CreateTimestampTests(unittest.TestCase):
    def test_timestamp_is_iso_utc_with_z_suffix(self) -> None:
        timestamp = monitor_lib._create_timestamp()

        self.assertTrue(timestamp.endswith("Z"))
        self.assertNotIn("+00:00", timestamp)
        self.assertIn("T", timestamp)


class SendWorkerTests(unittest.TestCase):
    def test_send_worker_connects_sends_and_closes(self) -> None:
        fake_socket = mock.MagicMock()

        with mock.patch.object(
            monitor_lib.socket, "socket", return_value=fake_socket
        ):
            monitor_lib._send_worker("mensagem\n")

        fake_socket.connect.assert_called_once_with(monitor_lib.SOCKET_PATH)
        fake_socket.sendall.assert_called_once_with(b"mensagem\n")
        fake_socket.close.assert_called_once()

    def test_send_worker_closes_socket_even_on_connect_failure(self) -> None:
        fake_socket = mock.MagicMock()
        fake_socket.connect.side_effect = ConnectionRefusedError("sem servidor")

        with mock.patch.object(
            monitor_lib.socket, "socket", return_value=fake_socket
        ):
            monitor_lib._send_worker("mensagem\n")

        fake_socket.sendall.assert_not_called()
        fake_socket.close.assert_called_once()


class SendToMonitorTests(unittest.TestCase):
    def test_send_to_monitor_dispatches_worker_in_thread(self) -> None:
        with mock.patch.object(monitor_lib, "threading") as fake_threading:
            monitor_lib.send_to_monitor("camera", {"frames": 10})

        fake_threading.Thread.assert_called_once()
        _, kwargs = fake_threading.Thread.call_args
        self.assertEqual(kwargs["target"], monitor_lib._send_worker)
        (message,) = kwargs["args"]
        payload = json.loads(message.strip())
        self.assertEqual(payload["component"], "camera")
        fake_threading.Thread.return_value.start.assert_called_once()


if __name__ == "__main__":
    unittest.main()

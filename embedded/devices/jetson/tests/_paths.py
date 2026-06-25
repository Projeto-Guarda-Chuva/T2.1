import os
import sys

_JETSON_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

for _subdir in ("client_lib", "monitor"):
    _path = os.path.join(_JETSON_DIR, _subdir)
    if _path not in sys.path:
        sys.path.insert(0, _path)

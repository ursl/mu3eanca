"""Base64 encoding/decoding utilities."""

import base64
from typing import Union


def encode_base64(data: Union[str, bytes]) -> str:
    """Encode data to base64 string."""
    if isinstance(data, str):
        data = data.encode('utf-8')
    return base64.b64encode(data).decode('utf-8')


def decode_base64(encoded: str) -> bytes:
    """Decode base64 string to bytes."""
    return base64.b64decode(encoded)


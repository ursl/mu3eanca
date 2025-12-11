"""BLOB encoding/decoding utilities.

Matches the C++ implementation in cdbUtil.cc.
BLOBs use 8-byte arrays for all types (int, uint, double).
"""

import struct
from typing import Iterator, List
from .base64 import decode_base64, encode_base64

# BLOB header magic number
BLOB_HEADER = 0xdeadface


def uint2blob(value: int) -> bytes:
    """
    Convert unsigned int to BLOB (8-byte array).
    
    Args:
        value: Unsigned integer value
    
    Returns:
        8-byte array (little-endian, padded to 8 bytes)
    """
    data = struct.pack("<I", value)  # 4 bytes, little-endian
    return data + b'\x00' * 4  # Pad to 8 bytes


def int2blob(value: int) -> bytes:
    """
    Convert int to BLOB (8-byte array).
    
    Args:
        value: Integer value
    
    Returns:
        8-byte array (little-endian, padded to 8 bytes)
    """
    data = struct.pack("<i", value)  # 4 bytes, little-endian
    return data + b'\x00' * 4  # Pad to 8 bytes


def double2blob(value: float) -> bytes:
    """
    Convert double to BLOB (8-byte array).
    
    Args:
        value: Double value
    
    Returns:
        8-byte array (little-endian)
    """
    return struct.pack("<d", value)  # 8 bytes, little-endian


def blob2uint(blob: bytes) -> int:
    """
    Convert BLOB to unsigned int.
    
    Args:
        blob: 8-byte array (only first 4 bytes used)
    
    Returns:
        Unsigned integer value
    """
    return struct.unpack("<I", blob[:4])[0]


def blob2int(blob: bytes) -> int:
    """
    Convert BLOB to int.
    
    Args:
        blob: 8-byte array (only first 4 bytes used)
    
    Returns:
        Integer value
    """
    return struct.unpack("<i", blob[:4])[0]


def blob2double(blob: bytes) -> float:
    """
    Convert BLOB to double.
    
    Args:
        blob: 8-byte array
    
    Returns:
        Double value
    """
    return struct.unpack("<d", blob[:8])[0]


def dump_array(data: bytes) -> str:
    """
    Dump binary data as string (for concatenation before base64 encoding).
    
    Args:
        data: 8-byte array
    
    Returns:
        String representation of bytes
    """
    return data.decode('latin-1')  # Preserve all byte values


def get_data(iterator: Iterator[int]) -> bytes:
    """
    Extract 8 bytes from iterator.
    
    Args:
        iterator: Iterator over bytes/characters
    
    Returns:
        8-byte array
    
    Raises:
        StopIteration: If not enough data available (less than 8 bytes)
    """
    data = bytearray()
    for _ in range(8):
        try:
            byte_val = next(iterator)
            if isinstance(byte_val, str):
                data.append(ord(byte_val))
            else:
                data.append(byte_val)
        except StopIteration:
            # If we didn't get 8 bytes, raise StopIteration
            if len(data) < 8:
                raise
            break
    return bytes(data)


def decode_blob_string(blob_string: str) -> bytes:
    """
    Decode base64-encoded BLOB string to binary.
    
    Args:
        blob_string: Base64-encoded BLOB string
    
    Returns:
        Binary BLOB data
    """
    return decode_base64(blob_string)


def encode_blob_string(blob_data: bytes) -> str:
    """
    Encode binary BLOB data to base64 string.
    
    Args:
        blob_data: Binary BLOB data
    
    Returns:
        Base64-encoded string
    """
    return encode_base64(blob_data)


def make_blob_header() -> bytes:
    """Create BLOB header (0xdeadface)."""
    return uint2blob(BLOB_HEADER)


def read_blob_header(iterator: Iterator[int]) -> int:
    """
    Read and verify BLOB header.
    
    Args:
        iterator: Iterator over bytes
    
    Returns:
        Header value (should be 0xdeadface)
    
    Raises:
        StopIteration: If not enough data for header
    """
    try:
        header_data = get_data(iterator)
        return blob2uint(header_data)
    except StopIteration:
        raise ValueError("Not enough data to read BLOB header (need at least 8 bytes)")


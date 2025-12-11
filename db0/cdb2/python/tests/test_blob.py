"""Tests for BLOB encoding/decoding utilities."""

import unittest
from mu3e_cdb.utils.blob import (
    uint2blob, int2blob, double2blob,
    blob2uint, blob2int, blob2double,
    make_blob_header, read_blob_header,
    get_data, dump_array,
    encode_blob_string, decode_blob_string,
)


class TestBlobUtilities(unittest.TestCase):
    """Test BLOB utility functions."""
    
    def test_uint_roundtrip(self):
        """Test unsigned int encoding/decoding."""
        value = 0xdeadface
        blob = uint2blob(value)
        decoded = blob2uint(blob)
        self.assertEqual(value, decoded)
    
    def test_int_roundtrip(self):
        """Test int encoding/decoding."""
        value = -12345
        blob = int2blob(value)
        decoded = blob2int(blob)
        self.assertEqual(value, decoded)
    
    def test_double_roundtrip(self):
        """Test double encoding/decoding."""
        value = 3.14159265359
        blob = double2blob(value)
        decoded = blob2double(blob)
        self.assertAlmostEqual(value, decoded, places=10)
    
    def test_blob_header(self):
        """Test BLOB header creation and reading."""
        header = make_blob_header()
        iterator = iter(header)
        decoded_header = read_blob_header(iterator)
        self.assertEqual(decoded_header, 0xdeadface)
    
    def test_get_data(self):
        """Test get_data function."""
        data = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a'
        iterator = iter(data)
        result = get_data(iterator)
        self.assertEqual(len(result), 8)
        self.assertEqual(result[:4], b'\x01\x02\x03\x04')
    
    def test_encode_decode_blob_string(self):
        """Test base64 encoding/decoding of BLOB."""
        original = b'\x01\x02\x03\x04\x05\x06\x07\x08'
        encoded = encode_blob_string(original)
        decoded = decode_blob_string(encoded)
        self.assertEqual(original, decoded)


if __name__ == '__main__':
    unittest.main()


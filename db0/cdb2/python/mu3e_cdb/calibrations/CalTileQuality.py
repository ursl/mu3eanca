"""Tile quality calibration class."""

from enum import IntEnum
from typing import Optional
from .base import Calibration
from ..database.base import Database


class TileQualityStatus(IntEnum):
    """Tile quality status enumeration."""
    CHANNEL_NOT_FOUND = -1
    GOOD = 0
    NOISY = 1
    DEAD = 2
    UNSET = 3
    DECLARED_BAD = 4
    MASKED = 9
    
    @classmethod
    def to_string(cls, status: 'TileQualityStatus') -> str:
        """Convert status to string."""
        status_map = {
            cls.CHANNEL_NOT_FOUND: "ChannelNotFound",
            cls.GOOD: "Good",
            cls.NOISY: "Noisy",
            cls.DEAD: "Dead",
            cls.UNSET: "Unset",
            cls.DECLARED_BAD: "DeclaredBad",
            cls.MASKED: "Masked",
        }
        return status_map.get(status, "Unknown")


class CalTileQuality(Calibration):
    """Tile quality calibration."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # channel_id -> quality
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "tilequality_"
    
    def calculate(self, hash: str) -> None:
        """Load and calculate calibration from hash."""
        if not self.db:
            raise ValueError("Database not set")
        
        payload = self.db.get_payload(hash)
        if not payload:
            if self.verbose > 0:
                print(f"Warning: Payload not found for hash: {hash}")
            self.hash = hash
            self._constants.clear()
            return
        
        # Check if BLOB is empty
        if not payload.blob or len(payload.blob.strip()) == 0:
            if self.verbose > 0:
                print(f"Warning: Payload {hash} has empty BLOB")
            self.hash = hash
            self._constants.clear()
            return
        
        self.hash = hash
        self._constants.clear()
        
        # Use schema from payload to decode BLOB dynamically
        schema = payload.schema
        if not schema:
            # Fallback to hardcoded schema if payload doesn't have one
            schema = self.get_schema()
            if self.verbose > 0:
                print(f"Warning: No schema in payload, using default: {schema}")
        
        # Decode base64 BLOB
        from ..utils.blob import decode_blob_string, read_blob_header
        from ..utils.schema import SchemaDecoder
        
        try:
            blob_binary = decode_blob_string(payload.blob)
        except Exception as e:
            raise ValueError(f"Failed to decode BLOB (base64): {e}")
        
        if len(blob_binary) < 8:
            if self.verbose > 0:
                print(f"Warning: BLOB too short: {len(blob_binary)} bytes (need at least 8 for header)")
            return
        
        # Read header
        blob_iter = iter(blob_binary)
        try:
            header = read_blob_header(blob_iter)
            if self.verbose > 0:
                print(f"CalTileQuality::calculate() with hash={hash}, header=0x{header:x}")
                print(f"  Using schema: {schema}")
        except (StopIteration, ValueError) as e:
            raise ValueError(f"Failed to read BLOB header: {e}")
        
        # Create schema decoder
        decoder = SchemaDecoder(schema)
        
        # Parse constants using schema
        while True:
            try:
                # Decode one record using the schema
                decoded = decoder.decode_blob(blob_binary, blob_iter)
                
                # Check if we got valid data (at least id field)
                if "id" not in decoded:
                    break
                
                channel_id = decoded["id"]
                quality = decoded.get("qual", -1)
                
                self._constants[channel_id] = {"quality": quality}
            except StopIteration:
                # End of data
                break
        
        if self.verbose > 0:
            print(f"CalTileQuality::calculate() inserted {len(self._constants)} constants")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_id,i_qual"
    
    def get_channel_quality(self, channel_id: int) -> TileQualityStatus:
        """Get channel quality status."""
        if channel_id not in self._constants:
            return TileQualityStatus.CHANNEL_NOT_FOUND
        return TileQualityStatus(self._constants[channel_id]["quality"])


"""Tile alignment calibration class."""

from typing import Optional, Dict
from .base import Calibration
from ..database.base import Database


class CalTileAlignment(Calibration):
    """Tile alignment calibration."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # tile_id -> alignment constants
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "tilealignment_"
    
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
                print(f"CalTileAlignment::calculate() with hash={hash}, header=0x{header:x}")
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
                
                tile_id = decoded["id"]
                self._constants[tile_id] = {
                    "sensor": decoded.get("sensor", -1),
                    "posx": decoded.get("posx", 0.0),
                    "posy": decoded.get("posy", 0.0),
                    "posz": decoded.get("posz", 0.0),
                    "dirx": decoded.get("dirx", 0.0),
                    "diry": decoded.get("diry", 0.0),
                    "dirz": decoded.get("dirz", 0.0),
                }
            except StopIteration:
                # End of data
                break
        
        if self.verbose > 0:
            print(f"CalTileAlignment::calculate() inserted {len(self._constants)} constants")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_id,i_sensor,posx,posy,posz,dirx,diry,dirz"
    
    def get_position(self, tile_id: int) -> Dict[str, float]:
        """Get position (posx, posy, posz) for tile."""
        const = self._constants.get(tile_id, {})
        return {
            "x": const.get("posx", 0.0),
            "y": const.get("posy", 0.0),
            "z": const.get("posz", 0.0),
        }
    
    def get_direction(self, tile_id: int) -> Dict[str, float]:
        """Get direction (dirx, diry, dirz) for tile."""
        const = self._constants.get(tile_id, {})
        return {
            "x": const.get("dirx", 0.0),
            "y": const.get("diry", 0.0),
            "z": const.get("dirz", 0.0),
        }
    
    def get_sensor(self, tile_id: int) -> int:
        """Get sensor number for tile."""
        return self._constants.get(tile_id, {}).get("sensor", -1)


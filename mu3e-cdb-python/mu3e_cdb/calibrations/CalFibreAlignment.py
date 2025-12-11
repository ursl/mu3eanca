"""Fibre alignment calibration class."""

from typing import Optional, Dict
from .base import Calibration
from ..database.base import Database


class CalFibreAlignment(Calibration):
    """Fibre alignment calibration."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # fibre_id -> alignment constants
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "fibrealignment_"
    
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
                print(f"CalFibreAlignment::calculate() with hash={hash}, header=0x{header:x}")
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
                
                fibre_id = decoded["id"]
                self._constants[fibre_id] = {
                    "cx": decoded.get("cx", 0.0),
                    "cy": decoded.get("cy", 0.0),
                    "cz": decoded.get("cz", 0.0),
                    "fx": decoded.get("fx", 0.0),
                    "fy": decoded.get("fy", 0.0),
                    "fz": decoded.get("fz", 0.0),
                    "round": bool(decoded.get("round", 0)),
                    "square": bool(decoded.get("square", 0)),
                    "diameter": decoded.get("diameter", 0.0),
                }
            except StopIteration:
                # End of data
                break
        
        if self.verbose > 0:
            print(f"CalFibreAlignment::calculate() inserted {len(self._constants)} constants")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_id,cx,cy,cz,fx,fy,fz,b_round,b_square,diameter"
    
    # Accessor methods
    def get_center(self, fibre_id: int) -> Dict[str, float]:
        """Get center position (cx, cy, cz) for fibre."""
        const = self._constants.get(fibre_id, {})
        return {
            "x": const.get("cx", 0.0),
            "y": const.get("cy", 0.0),
            "z": const.get("cz", 0.0),
        }
    
    def get_front(self, fibre_id: int) -> Dict[str, float]:
        """Get front position (fx, fy, fz) for fibre."""
        const = self._constants.get(fibre_id, {})
        return {
            "x": const.get("fx", 0.0),
            "y": const.get("fy", 0.0),
            "z": const.get("fz", 0.0),
        }
    
    def is_round(self, fibre_id: int) -> bool:
        """Check if fibre is round."""
        return self._constants.get(fibre_id, {}).get("round", False)
    
    def is_square(self, fibre_id: int) -> bool:
        """Check if fibre is square."""
        return self._constants.get(fibre_id, {}).get("square", False)
    
    def get_diameter(self, fibre_id: int) -> float:
        """Get fibre diameter."""
        return self._constants.get(fibre_id, {}).get("diameter", 0.0)


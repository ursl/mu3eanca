"""MPPC alignment calibration class."""

from typing import Optional, Dict
from .base import Calibration
from ..database.base import Database


class CalMppcAlignment(Calibration):
    """MPPC alignment calibration."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # mppc_id -> alignment constants
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "mppcalignment_"
    
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
                print(f"CalMppcAlignment::calculate() with hash={hash}, header=0x{header:x}")
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
                
                # Check if we got valid data (at least mppc field)
                if "mppc" not in decoded:
                    break
                
                mppc_id = decoded["mppc"]
                self._constants[mppc_id] = {
                    "vx": decoded.get("vx", 0.0),
                    "vy": decoded.get("vy", 0.0),
                    "vz": decoded.get("vz", 0.0),
                    "colx": decoded.get("colx", 0.0),
                    "coly": decoded.get("coly", 0.0),
                    "colz": decoded.get("colz", 0.0),
                    "ncol": decoded.get("ncol", 0),
                }
            except StopIteration:
                # End of data
                break
        
        if self.verbose > 0:
            print(f"CalMppcAlignment::calculate() inserted {len(self._constants)} constants")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_mppc,vx,vy,vz,colx,coly,colz,i_ncol"
    
    def get_v(self, mppc_id: int) -> Dict[str, float]:
        """Get v vector (vx, vy, vz) for MPPC."""
        const = self._constants.get(mppc_id, {})
        return {
            "x": const.get("vx", 0.0),
            "y": const.get("vy", 0.0),
            "z": const.get("vz", 0.0),
        }
    
    def get_col(self, mppc_id: int) -> Dict[str, float]:
        """Get column vector (colx, coly, colz) for MPPC."""
        const = self._constants.get(mppc_id, {})
        return {
            "x": const.get("colx", 0.0),
            "y": const.get("coly", 0.0),
            "z": const.get("colz", 0.0),
        }
    
    def get_ncol(self, mppc_id: int) -> int:
        """Get number of columns for MPPC."""
        return self._constants.get(mppc_id, {}).get("ncol", 0)


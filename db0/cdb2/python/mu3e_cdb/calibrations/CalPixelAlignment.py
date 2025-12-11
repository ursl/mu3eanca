"""Pixel alignment calibration class."""

from typing import Optional, Dict
from .base import Calibration
from ..database.base import Database


class CalPixelAlignment(Calibration):
    """Pixel alignment calibration."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # sensor_id -> alignment constants
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "pixelalignment_"
    
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
                print(f"CalPixelAlignment::calculate() with hash={hash}, header=0x{header:x}")
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
                
                sensor_id = decoded["id"]
                self._constants[sensor_id] = {
                    "vx": decoded.get("vx", 0.0),
                    "vy": decoded.get("vy", 0.0),
                    "vz": decoded.get("vz", 0.0),
                    "rowx": decoded.get("rowx", 0.0),
                    "rowy": decoded.get("rowy", 0.0),
                    "rowz": decoded.get("rowz", 0.0),
                    "colx": decoded.get("colx", 0.0),
                    "coly": decoded.get("coly", 0.0),
                    "colz": decoded.get("colz", 0.0),
                    "nrow": decoded.get("nrow", 0),
                    "ncol": decoded.get("ncol", 0),
                    "width": decoded.get("width", 0.0),
                    "length": decoded.get("length", 0.0),
                    "thickness": decoded.get("thickness", 0.0),
                    "pixelSize": decoded.get("pixelsize", 0.0),
                }
            except StopIteration:
                # End of data
                break
        
        if self.verbose > 0:
            print(f"CalPixelAlignment::calculate() inserted {len(self._constants)} constants")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_id,vx,vy,vz,rowx,rowy,rowz,colx,coly,colz,i_nrow,i_ncol,width,length,thickness,pixelsize"
    
    # Accessor methods
    def get_vx(self, sensor_id: int) -> float:
        """Get vx for sensor."""
        return self._constants.get(sensor_id, {}).get("vx", 0.0)
    
    def get_vy(self, sensor_id: int) -> float:
        """Get vy for sensor."""
        return self._constants.get(sensor_id, {}).get("vy", 0.0)
    
    def get_vz(self, sensor_id: int) -> float:
        """Get vz for sensor."""
        return self._constants.get(sensor_id, {}).get("vz", 0.0)
    
    def get_row(self, sensor_id: int) -> Dict[str, float]:
        """Get row vector (rowx, rowy, rowz) for sensor."""
        const = self._constants.get(sensor_id, {})
        return {
            "x": const.get("rowx", 0.0),
            "y": const.get("rowy", 0.0),
            "z": const.get("rowz", 0.0),
        }
    
    def get_col(self, sensor_id: int) -> Dict[str, float]:
        """Get column vector (colx, coly, colz) for sensor."""
        const = self._constants.get(sensor_id, {})
        return {
            "x": const.get("colx", 0.0),
            "y": const.get("coly", 0.0),
            "z": const.get("colz", 0.0),
        }


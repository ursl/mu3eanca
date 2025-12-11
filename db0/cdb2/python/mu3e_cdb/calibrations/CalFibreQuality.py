"""Fibre quality calibration class."""

from enum import IntEnum
from typing import Optional
from .base import Calibration
from ..database.base import Database


class FibreQualityStatus(IntEnum):
    """Fibre quality status enumeration."""
    CHANNEL_NOT_FOUND = -1
    GOOD = 0
    NOISY = 1
    DEAD = 2
    UNLOCKED = 3
    HAS_NO_DATA = 4
    UNSET = 5


class CalFibreQuality(Calibration):
    """Fibre quality calibration."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # asic_id -> constants dict
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "fibrequality_"
    
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
                print(f"CalFibreQuality::calculate() with hash={hash}, header=0x{header:x}")
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
                
                asic_id = decoded["id"]
                quality = decoded.get("quality", -1)
                lock = decoded.get("lock", 0)
                has_data = decoded.get("hasData", 0)
                threshold = decoded.get("thr", 0.0)
                efficiency = decoded.get("eff", 0.0)
                
                self._constants[asic_id] = {
                    "quality": quality,
                    "lock": bool(lock),
                    "hasData": bool(has_data),
                    "threshold": threshold,
                    "efficiency": efficiency,
                }
            except StopIteration:
                # End of data
                break
        
        if self.verbose > 0:
            print(f"CalFibreQuality::calculate() inserted {len(self._constants)} constants")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_id,i_quality,i_lock,i_hasData,thr,eff"
    
    def get_asic_status(self, asic_id: int) -> FibreQualityStatus:
        """Get ASIC status."""
        if asic_id not in self._constants:
            return FibreQualityStatus.CHANNEL_NOT_FOUND
        
        const = self._constants[asic_id]
        lock = const.get("lock", False)
        has_data = const.get("hasData", False)
        
        if lock and has_data:
            return FibreQualityStatus.GOOD
        elif not lock:
            return FibreQualityStatus.UNLOCKED
        elif not has_data:
            return FibreQualityStatus.HAS_NO_DATA
        
        return FibreQualityStatus.UNSET
    
    def get_asic_lock(self, asic_id: int) -> bool:
        """Get ASIC lock status."""
        if asic_id not in self._constants:
            return False
        return self._constants[asic_id].get("lock", False)
    
    def get_asic_has_data(self, asic_id: int) -> bool:
        """Get ASIC has_data status."""
        if asic_id not in self._constants:
            return False
        return self._constants[asic_id].get("hasData", False)
    
    def get_asic_quality(self, asic_id: int) -> int:
        """Get ASIC quality."""
        if asic_id not in self._constants:
            return -1
        return self._constants[asic_id].get("quality", -1)
    
    def get_asic_threshold(self, asic_id: int) -> float:
        """Get ASIC threshold."""
        if asic_id not in self._constants:
            return -1.0
        return self._constants[asic_id].get("threshold", -1.0)
    
    def get_asic_efficiency(self, asic_id: int) -> float:
        """Get ASIC efficiency."""
        if asic_id not in self._constants:
            return -1.0
        return self._constants[asic_id].get("efficiency", -1.0)


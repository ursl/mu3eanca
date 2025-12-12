"""Pixel quality calibration class."""

from enum import IntEnum
from typing import Optional
from .base import Calibration
from ..database.base import Database


class PixelQualityStatus(IntEnum):
    """Pixel quality status enumeration."""
    CHIP_NOT_FOUND = -1
    GOOD = 0
    NOISY = 1
    SUSPECT = 2
    DECLARED_BAD = 3
    LVDS_ERROR_LINK = 4
    LVDS_ERROR_OTHER_LINK = 5
    LVDS_ERROR_TOP_BOTTOM_EDGE = 6
    DEAD_CHIP = 7
    NO_HITS = 8
    MASKED = 9
    
    @classmethod
    def to_string(cls, status: 'PixelQualityStatus') -> str:
        """Convert status to string."""
        status_map = {
            cls.CHIP_NOT_FOUND: "ChipNotFound",
            cls.GOOD: "Good",
            cls.NOISY: "Noisy",
            cls.SUSPECT: "Suspect",
            cls.DECLARED_BAD: "DeclaredBad",
            cls.LVDS_ERROR_LINK: "LVDSErrorLink",
            cls.LVDS_ERROR_OTHER_LINK: "LVDSErrorOtherLink",
            cls.LVDS_ERROR_TOP_BOTTOM_EDGE: "LVDSErrorTopBottomEdge",
            cls.DEAD_CHIP: "DeadChip",
            cls.NO_HITS: "NoHits",
            cls.MASKED: "Masked",
        }
        return status_map.get(status, "Unknown")
    
    @classmethod
    def get_documentation(cls) -> str:
        """Get documentation string for all status values."""
        all_statuses = [
            cls.CHIP_NOT_FOUND,
            cls.GOOD,
            cls.NOISY,
            cls.SUSPECT,
            cls.DECLARED_BAD,
            cls.LVDS_ERROR_LINK,
            cls.LVDS_ERROR_OTHER_LINK,
            cls.LVDS_ERROR_TOP_BOTTOM_EDGE,
            cls.DEAD_CHIP,
            cls.NO_HITS,
            cls.MASKED,
        ]
        
        doc_parts = [f"{int(s)}={cls.to_string(s)}" for s in all_statuses]
        return ", ".join(doc_parts) + ". M-link=nhit/ovfl"


class CalPixelQualityLM(Calibration):
    """Pixel quality calibration using 3+1 link words (matches C++ calPixelQualityLM)."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        super().__init__(db, tag)
        self._constants = {}  # chip_id -> constants dict
    
    def get_name(self) -> str:
        """Return the calibration name."""
        return "pixelqualitylm_"
    
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
                print(f"CalPixelQualityLM::calculate() with hash={hash}, header=0x{header:x}")
                print(f"  Using schema: {schema}")
                print(f"  BLOB size: {len(blob_binary)} bytes")
        except (StopIteration, ValueError) as e:
            raise ValueError(f"Failed to read BLOB header: {e}")
        
        # Create schema decoder
        decoder = SchemaDecoder(schema)
        
        # Parse constants using schema
        record_count = 0
        # Parse constants using schema
        record_count = 0
        while True:
            try:
                # Decode one record using the schema
                decoded = decoder.decode_blob(blob_binary, blob_iter)
                
                # Check if we got valid data (at least id field)
                if "id" not in decoded:
                    if self.verbose > 1:
                        print(f"  No 'id' field in decoded record {record_count + 1}")
                    break
                
                chip_id = decoded["id"]
                record_count += 1
                
                if self.verbose > 0 and record_count <= 15:
                    print(f"  Record {record_count}: chip_id={chip_id}, linkA={decoded.get('linkA')}, linkB={decoded.get('linkB')}, linkC={decoded.get('linkC')}, linkM={decoded.get('linkM')}, ncol={decoded.get('ncol', 'MISSING')}, npix={decoded.get('npix', 'MISSING')}")
                
                # Extract fields
                mcol = {}
                if "ncol" in decoded and decoded["ncol"] > 0:
                    ncol_data = decoded.get("ncol_data", [])
                    if self.verbose > 0 and record_count <= 5:
                        print(f"    ncol_data has {len(ncol_data)} items")
                    for item in ncol_data:
                        if "col" in item and "qual" in item:
                            mcol[item["col"]] = item["qual"]
                
                mpixel = {}
                if "npix" in decoded and decoded["npix"] > 0:
                    npix_data = decoded.get("npix_data", [])
                    if self.verbose > 0 and record_count <= 5:
                        print(f"    npix_data has {len(npix_data)} items (expected {decoded['npix']})")
                    for item in npix_data:
                        if "col" in item and "row" in item and "qual" in item:
                            idx = item["col"] * 250 + item["row"]
                            mpixel[idx] = item["qual"]
                
                try:
                    self._constants[chip_id] = {
                        "ckdivend": decoded.get("ckdivend", 0),
                        "ckdivend2": decoded.get("ckdivend2", 0),
                        "linkA": decoded.get("linkA", 0),
                        "linkB": decoded.get("linkB", 0),
                        "linkC": decoded.get("linkC", 0),
                        "linkM": decoded.get("linkM", 0),
                        "mcol": mcol,
                        "mpixel": mpixel,
                    }
                    if self.verbose > 0 and record_count <= 10:
                        print(f"    Successfully inserted chip {chip_id}")
                except Exception as e:
                    if self.verbose > 0:
                        print(f"    ERROR inserting chip {chip_id}: {e}")
                        import traceback
                        traceback.print_exc()
                    raise
            except StopIteration:
                # End of data - no more bytes available
                if self.verbose > 0:
                    print(f"  Reached end of BLOB data after {record_count} records")
                break
            except (KeyError, ValueError) as e:
                # Parsing error
                if self.verbose > 0:
                    print(f"  Error parsing record {record_count + 1}: {e}")
                    import traceback
                    traceback.print_exc()
                break
            except Exception as e:
                # Unexpected error
                if self.verbose > 0:
                    print(f"  Unexpected error parsing record {record_count + 1}: {e}")
                    import traceback
                    traceback.print_exc()
                break
        
        if self.verbose > 0:
            print(f"CalPixelQualityLM::calculate() inserted {len(self._constants)} constants")
            if len(self._constants) > 0:
                # Print first few chip IDs
                chip_ids = sorted(self._constants.keys())[:10]
                print(f"  Sample chip IDs: {chip_ids}")
                if 36 in self._constants:
                    chip36 = self._constants[36]
                    print(f"  Chip 36 found: linkA={chip36.get('linkA')}, linkB={chip36.get('linkB')}, linkC={chip36.get('linkC')}, linkM={chip36.get('linkM')}")
                else:
                    print(f"  Chip 36 NOT found in decoded constants!")
    
    def get_schema(self) -> str:
        """Return the schema string."""
        return "ui_id,ui_ckdivend,ui_ckdivend2,ui_linkA,ui_linkB,ui_linkC,ui_linkM,i_ncol[,i_col,ui_qual],i_npix[,i_col,i_row,ui_qual]"
    
    def get_status(self, chip_id: int, col: int, row: int) -> PixelQualityStatus:
        """Get pixel status for a given chip, column, and row."""
        if chip_id not in self._constants:
            return PixelQualityStatus.CHIP_NOT_FOUND
        
        const = self._constants[chip_id]
        
        # First check dead links (matches C++ implementation)
        if const["linkA"] > 0 and 0 <= col <= 87:
            return PixelQualityStatus(const["linkA"])
        if const["linkB"] > 0 and 88 <= col <= 171:
            return PixelQualityStatus(const["linkB"])
        if const["linkC"] > 0 and 172 <= col <= 255:
            return PixelQualityStatus(const["linkC"])
        
        # Now check column status
        if col in const["mcol"]:
            return PixelQualityStatus(const["mcol"][col])
        
        # Finally check pixel status
        idx = col * 250 + row
        if idx in const["mpixel"]:
            return PixelQualityStatus(const["mpixel"][idx])
        
        # Default to good
        return PixelQualityStatus.GOOD
    
    def get_col_status(self, chip_id: int, col: int) -> PixelQualityStatus:
        """Get column status for a given chip and column."""
        if chip_id not in self._constants:
            return PixelQualityStatus.CHIP_NOT_FOUND
        
        const = self._constants[chip_id]
        if col in const["mcol"]:
            return PixelQualityStatus(const["mcol"][col])
        
        return PixelQualityStatus.GOOD
    
    def get_link_status(self, chip_id: int, link: int) -> PixelQualityStatus:
        """Get link status for a given chip and link (0-3)."""
        if chip_id not in self._constants:
            return PixelQualityStatus.CHIP_NOT_FOUND
        
        const = self._constants[chip_id]
        link_map = {0: "linkA", 1: "linkB", 2: "linkC", 3: "linkM"}
        if link in link_map:
            link_value = const[link_map[link]]
            if link_value == 0:
                return PixelQualityStatus.GOOD
            return PixelQualityStatus(link_value)
        
        return PixelQualityStatus.CHIP_NOT_FOUND
    
    def is_chip_dead(self, chip_id: int, row: int = -1, col: int = -1) -> bool:
        """Check if chip is dead (all three links are dead: DeadChip, NoHits, or Masked)."""
        if chip_id not in self._constants:
            return True
        
        # Check if all three links are dead (status 7, 8, or 9)
        cnt_dead_links = 0
        for ilink in range(3):
            if self.is_link_dead(chip_id, ilink):
                cnt_dead_links += 1
        return cnt_dead_links == 3
    
    def is_link_bad(self, chip_id: int, link: int) -> bool:
        """Check if link is bad."""
        status = self.get_link_status(chip_id, link)
        return status != PixelQualityStatus.GOOD
    
    def is_link_dead(self, chip_id: int, link: int) -> bool:
        """Check if link is dead (DeadChip=7, NoHits=8, or Masked=9)."""
        status = self.get_link_status(chip_id, link)
        return status in (PixelQualityStatus.DEAD_CHIP, PixelQualityStatus.NO_HITS, PixelQualityStatus.MASKED)
    
    def get_lvds_overflow_rate(self, chip_id: int) -> float:
        """Get LVDS overflow rate for a chip."""
        if chip_id not in self._constants:
            return -1.0
        
        const = self._constants[chip_id]
        link_m = const.get("linkM", 0)
        
        if link_m == 0:
            return 0.0  # No overflow
        elif link_m == 1:
            return 9.99e99  # Overflow rate = 0 (sentinel value)
        else:
            return 1.0 / float(link_m)
    
    def get_ckdivend(self, chip_id: int) -> int:
        """Get ckdivend for a chip."""
        if chip_id not in self._constants:
            return -1
        return self._constants[chip_id].get("ckdivend", 0)
    
    def get_ckdivend2(self, chip_id: int) -> int:
        """Get ckdivend2 for a chip."""
        if chip_id not in self._constants:
            return -1
        return self._constants[chip_id].get("ckdivend2", 0)
    
    def get_npix_with_status(self, chip_id: int, status: PixelQualityStatus) -> int:
        """Get number of pixels with given status."""
        if chip_id not in self._constants:
            return 0
        
        # Implementation would count pixels with given status
        return 0


"""Main entry point for Mu3e Conditions Database."""

from typing import Optional, List, Dict
from .database.base import Database
from .calibrations.base import Calibration
from .calibrations.CalPixelQualityLM import CalPixelQualityLM
from .calibrations.CalPixelAlignment import CalPixelAlignment
from .calibrations.CalFibreQuality import CalFibreQuality
from .calibrations.CalFibreAlignment import CalFibreAlignment
from .calibrations.CalTileQuality import CalTileQuality
from .calibrations.CalTileAlignment import CalTileAlignment
from .calibrations.CalMppcAlignment import CalMppcAlignment
from .models.run_record import RunRecord
from .models.config import ConfigPayload


class Mu3eConditions:
    """
    Main entry point for accessing Mu3e conditions database.
    
    This is a singleton class that manages access to calibration data,
    run records, and configurations.
    
    Example:
        >>> from mu3e_cdb import Mu3eConditions, JsonDatabase
        >>> 
        >>> # Initialize with JSON database
        >>> db = JsonDatabase("/path/to/json/db")
        >>> conditions = Mu3eConditions.instance("datav6.3=2025V0", db)
        >>> 
        >>> # Set run number
        >>> conditions.set_run_number(3265)
        >>> 
        >>> # Get calibration
        >>> pixel_quality = conditions.get_calibration("pixelqualitylm_")
        >>> status = pixel_quality.get_status(chip_id=42, col=10, row=20)
    """
    
    _instance: Optional['Mu3eConditions'] = None
    
    def __init__(self, global_tag: str = "unset", db: Optional[Database] = None):
        self._global_tag = global_tag
        self._db = db
        self._run_number = -1
        self._verbose = 0
        self._print_timing = 0
        
        # Cache for metadata
        self._global_tags: List[str] = []
        self._tags: List[str] = []
        self._iovs: Dict[str, List[int]] = {}
        
        # Cache for calibration objects
        self._calibrations: Dict[str, Calibration] = {}
        
        # Initialize if database is provided
        if self._db:
            self._initialize()
    
    @classmethod
    def instance(cls, global_tag: str = "unset", db: Optional[Database] = None) -> 'Mu3eConditions':
        """
        Get singleton instance of Mu3eConditions.
        
        Args:
            global_tag: Global tag to use (e.g., "datav6.3=2025V0")
            db: Database instance (JsonDatabase, MongoDatabase, etc.)
        
        Returns:
            Mu3eConditions instance
        """
        if cls._instance is None:
            cls._instance = cls(global_tag, db)
        return cls._instance
    
    def _initialize(self) -> None:
        """Initialize metadata from database."""
        if not self._db:
            return
        
        self._global_tags = self._db.read_global_tags()
        if self._global_tag != "unset":
            self._tags = self._db.read_tags(self._global_tag)
            self._iovs = self._db.read_iovs(self._tags)
    
    def use_cdb(self) -> bool:
        """Check if CDB is available."""
        return self._db is not None
    
    def get_global_tag(self) -> str:
        """Get current global tag."""
        return self._global_tag
    
    def get_global_tags(self) -> List[str]:
        """Get all available global tags."""
        return self._global_tags.copy()
    
    def get_tags(self, filter_pattern: str = "unset") -> List[str]:
        """
        Get tags matching filter pattern.
        
        Args:
            filter_pattern: Optional filter pattern (default: "unset" for all)
        
        Returns:
            List of tag names
        """
        if filter_pattern == "unset":
            return self._tags.copy()
        
        # Filter tags by pattern
        return [tag for tag in self._tags if filter_pattern in tag]
    
    def get_iovs(self, tag: str) -> List[int]:
        """
        Get IOVs (Intervals of Validity) for a tag.
        
        Args:
            tag: Tag name
        
        Returns:
            List of IOV run numbers
        """
        return self._iovs.get(tag, []).copy()
    
    def set_run_number(self, run: int) -> None:
        """
        Set the run number for IOV resolution.
        
        Args:
            run: Run number
        """
        self._run_number = run
        # Update calibrations when run number changes
        for cal in self._calibrations.values():
            self._update_calibration_for_run(cal, run)
    
    def get_run_number(self) -> int:
        """Get current run number."""
        return self._run_number
    
    def _update_calibration_for_run(self, cal: Calibration, run: int) -> None:
        """Update calibration object for a given run."""
        if not self._db or not cal.tag:
            return
        
        # Use the full tag name (e.g., "pixelqualitylm_datav6.3=2025V0")
        full_tag = cal.tag
        iovs = self.get_iovs(full_tag)
        
        if not iovs:
            if self._verbose > 0:
                print(f"Warning: No IOVs found for tag '{full_tag}'")
            return
        
        # Find the IOV that covers this run
        valid_iov = -1
        for iov in sorted(iovs):
            if run >= iov:
                valid_iov = iov
            else:
                break
        
        if valid_iov >= 0:
            # Construct hash: tag_<full_tag>_iov_<iov>
            # Example: tag_pixelqualitylm_datav6.3=2025V0_iov_1
            hash = f"tag_{full_tag}_iov_{valid_iov}"
            if self._verbose > 0:
                print(f"Loading calibration with hash: {hash}")
            cal.calculate(hash)
        else:
            if self._verbose > 0:
                print(f"Warning: No valid IOV found for run {run} in tag '{full_tag}'")
    
    def get_calibration(self, name: str) -> Optional[Calibration]:
        """
        Get calibration object by name.
        
        Args:
            name: Calibration name (e.g., "pixelqualitylm_", "pixelalignment_")
        
        Returns:
            Calibration object or None if not found
        """
        # Check cache first
        if name in self._calibrations:
            return self._calibrations[name]
        
        # Create new calibration object
        try:
            cal = self._create_calibration(name)
            if self._verbose > 1:
                print(f"get_calibration: Created calibration '{name}': {cal}")
            if cal is not None:  # Use 'is not None' instead of truthiness check (cal can be empty but valid)
                # Set verbosity on calibration object
                cal.set_verbosity(self._verbose)
                # Update for current run if set (before caching, so we can catch errors)
                if self._run_number >= 0:
                    try:
                        self._update_calibration_for_run(cal, self._run_number)
                    except Exception as e:
                        if self._verbose > 0:
                            print(f"Warning: Failed to update calibration '{name}' for run {self._run_number}: {e}")
                        import traceback
                        if self._verbose > 1:
                            traceback.print_exc()
                        # Still return the calibration object even if update failed
                # Cache the calibration object
                self._calibrations[name] = cal
                if self._verbose > 1:
                    print(f"get_calibration: Cached calibration '{name}', returning: {cal}")
            else:
                if self._verbose > 0:
                    print(f"get_calibration: _create_calibration returned None for '{name}'")
            return cal
        except Exception as e:
            if self._verbose > 0:
                print(f"Error creating calibration '{name}': {e}")
            import traceback
            if self._verbose > 0:
                traceback.print_exc()
            return None
    
    def _create_calibration(self, name: str) -> Optional[Calibration]:
        """Create calibration object by name."""
        if self._verbose > 1:
            print(f"_create_calibration: called with name='{name}', _tags has {len(self._tags)} tags")
        # Find matching tag
        tag = None
        for t in self._tags:
            if name in t:
                tag = t
                if self._verbose > 1:
                    print(f"_create_calibration: Found matching tag: {tag}")
                break
        
        if not tag:
            if self._verbose > 0:
                print(f"Warning: Tag not found for calibration '{name}'")
                print(f"  Available tags: {self._tags}")
            return None
        
        # Create appropriate calibration class
        try:
            if name == "pixelqualitylm_":
                return CalPixelQualityLM(self._db, tag)
            elif name == "pixelalignment_":
                return CalPixelAlignment(self._db, tag)
            elif name == "fibrequality_":
                return CalFibreQuality(self._db, tag)
            elif name == "fibrealignment_":
                return CalFibreAlignment(self._db, tag)
            elif name == "tilequality_":
                return CalTileQuality(self._db, tag)
            elif name == "tilealignment_":
                return CalTileAlignment(self._db, tag)
            elif name == "mppcalignment_":
                return CalMppcAlignment(self._db, tag)
            
            if self._verbose > 0:
                print(f"Warning: Unknown calibration type '{name}'")
            return None
        except Exception as e:
            if self._verbose > 0:
                print(f"Error instantiating calibration '{name}': {e}")
            import traceback
            if self._verbose > 0:
                traceback.print_exc()
            return None
    
    def get_run_record(self, run_number: int) -> Optional[RunRecord]:
        """
        Get run record for a given run number.
        
        Args:
            run_number: Run number
        
        Returns:
            RunRecord or None if not found
        """
        if not self._db:
            return None
        return self._db.get_run_record(run_number)
    
    def get_all_run_numbers(self, selection: str = "", detector: str = "") -> List[str]:
        """
        Get all run numbers, optionally filtered.
        
        Args:
            selection: Optional class selection filter
            detector: Optional detector filter (comma-separated: "vtx,pix,fib,til")
        
        Returns:
            List of run number strings
        """
        if not self._db:
            return []
        return self._db.get_all_run_numbers(selection, detector)
    
    def get_config_string(self, cfg_name: str) -> Optional[str]:
        """
        Get configuration string by name.
        
        Args:
            cfg_name: Configuration name
        
        Returns:
            Configuration string or None
        """
        # Implementation would look up config by name
        return None
    
    def get_config_string_with_hash(self, cfg_hash: str) -> Optional[str]:
        """
        Get configuration string by hash.
        
        Args:
            cfg_hash: Configuration hash
        
        Returns:
            Configuration string or None
        """
        if not self._db:
            return None
        
        config = self._db.get_config(cfg_hash)
        if config:
            return config.cfg_string
        return None
    
    def get_hash(self, run: int, tag: str) -> Optional[str]:
        """
        Get hash for a given run and tag.
        
        Args:
            run: Run number
            tag: Tag name
        
        Returns:
            Hash string or None
        """
        iovs = self.get_iovs(tag)
        if not iovs:
            return None
        
        # Find the IOV that covers this run
        valid_iov = -1
        for iov in sorted(iovs):
            if run >= iov:
                valid_iov = iov
            else:
                break
        
        if valid_iov < 0:
            return None
        
        # Construct hash: tag_<full_tag>_iov_<iov>
        return f"tag_{tag}_iov_{valid_iov}"
    
    def which_iov(self, run: int, tag: str) -> int:
        """
        Determine which IOV covers a given run.
        
        Args:
            run: Run number
            tag: Tag name
        
        Returns:
            IOV run number or -1 if not found
        """
        iovs = self.get_iovs(tag)
        if not iovs:
            return -1
        
        # Find the IOV that covers this run
        valid_iov = -1
        for iov in sorted(iovs):
            if run >= iov:
                valid_iov = iov
            else:
                break
        
        return valid_iov
    
    def set_verbosity(self, v: int) -> None:
        """Set verbosity level."""
        self._verbose = v
        if self._db:
            self._db.verbose = v
        for cal in self._calibrations.values():
            cal.set_verbosity(v)
    
    def set_print_timing(self, v: int) -> None:
        """Set print timing flag."""
        self._print_timing = v
    
    def get_db(self) -> Optional[Database]:
        """Get database instance."""
        return self._db
    
    def set_db(self, db: Database) -> None:
        """Set database instance."""
        self._db = db
        self._initialize()


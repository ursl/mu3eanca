"""Factory for creating calibration classes."""

from typing import Optional
from .database.base import Database
from .calibrations.base import Calibration
from .calibrations.CalPixelQualityLM import CalPixelQualityLM
from .calibrations.CalPixelAlignment import CalPixelAlignment
from .calibrations.CalFibreQuality import CalFibreQuality
from .calibrations.CalFibreAlignment import CalFibreAlignment
from .calibrations.CalTileQuality import CalTileQuality
from .calibrations.CalTileAlignment import CalTileAlignment
from .calibrations.CalMppcAlignment import CalMppcAlignment


class CalibrationFactory:
    """Factory for creating calibration objects."""
    
    @staticmethod
    def create(name: str, db: Optional[Database] = None, tag: Optional[str] = None) -> Optional[Calibration]:
        """
        Create a calibration object by name.
        
        Args:
            name: Calibration name (e.g., "pixelqualitylm_", "pixelalignment_")
            db: Optional database instance
            tag: Optional tag name
        
        Returns:
            Calibration object or None if name is unknown
        """
        name_map = {
            "pixelqualitylm_": CalPixelQualityLM,
            "pixelalignment_": CalPixelAlignment,
            "fibrequality_": CalFibreQuality,
            "fibrealignment_": CalFibreAlignment,
            "tilequality_": CalTileQuality,
            "tilealignment_": CalTileAlignment,
            "mppcalignment_": CalMppcAlignment,
        }
        
        cal_class = name_map.get(name)
        if cal_class:
            return cal_class(db, tag)
        
        return None
    
    @staticmethod
    def create_from_file(hash: str, directory: str) -> Optional[Calibration]:
        """
        Create calibration object from file.
        
        Args:
            hash: Hash/name of calibration
            directory: Directory containing payload files
        
        Returns:
            Calibration object or None
        """
        # Extract calibration name from hash
        # Hash format: tag_<name>_<gt>_iov_<iov>
        if hash.startswith("tag_"):
            parts = hash.split("_")
            if len(parts) >= 2:
                name = parts[1] + "_"
            else:
                name = hash
        else:
            name = hash
        
        cal = CalibrationFactory.create(name)
        if cal:
            # Load from file
            # This would need to be implemented
            pass
        
        return cal


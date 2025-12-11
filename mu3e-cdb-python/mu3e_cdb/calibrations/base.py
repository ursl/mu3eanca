"""Base calibration class."""

from abc import ABC, abstractmethod
from typing import Optional, Iterator, Tuple, Any
from ..database.base import Database


class Calibration(ABC):
    """Abstract base class for all calibration objects."""
    
    def __init__(self, db: Optional[Database] = None, tag: Optional[str] = None):
        self.db = db
        self.tag = tag
        self.hash = "unset"
        self.verbose = 0
        self._constants = {}  # id -> constants dict (subclasses should initialize this)
    
    @abstractmethod
    def get_name(self) -> str:
        """Return the calibration name/tag prefix."""
        pass
    
    @abstractmethod
    def calculate(self, hash: str) -> None:
        """Load and calculate calibration from hash."""
        pass
    
    @abstractmethod
    def get_schema(self) -> str:
        """Return the schema string."""
        pass
    
    def set_verbosity(self, v: int) -> None:
        """Set verbosity level."""
        self.verbose = v
    
    def __iter__(self) -> Iterator[Tuple[int, dict]]:
        """Iterate over all constants (id, constants_dict) pairs."""
        return iter(self._constants.items())
    
    def get_all_ids(self):
        """Get all IDs in the calibration."""
        return list(self._constants.keys())
    
    def __len__(self) -> int:
        """Return the number of constants."""
        return len(self._constants)


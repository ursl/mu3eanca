"""Abstract base class for database implementations."""

from abc import ABC, abstractmethod
from typing import List, Dict, Optional
from ..models.payload import Payload
from ..models.run_record import RunRecord
from ..models.config import ConfigPayload


class Database(ABC):
    """Abstract base class for CDB database access."""
    
    def __init__(self, uri: str, verbose: int = 0):
        self.uri = uri
        self.verbose = verbose
        self.name = "Database"
    
    @abstractmethod
    def read_global_tags(self) -> List[str]:
        """Read all available global tags from database."""
        pass
    
    @abstractmethod
    def read_tags(self, global_tag: str) -> List[str]:
        """Read all tags for a given global tag."""
        pass
    
    @abstractmethod
    def read_iovs(self, tags: List[str]) -> Dict[str, List[int]]:
        """Read IOVs (Intervals of Validity) for given tags."""
        pass
    
    @abstractmethod
    def get_payload(self, hash: str) -> Optional[Payload]:
        """Get payload by hash."""
        pass
    
    @abstractmethod
    def get_run_record(self, run_number: int) -> Optional[RunRecord]:
        """Get run record for a given run number."""
        pass
    
    @abstractmethod
    def get_all_run_numbers(self, selection: str = "", detector: str = "") -> List[str]:
        """Get all run numbers, optionally filtered."""
        pass
    
    @abstractmethod
    def get_config(self, hash: str) -> Optional[ConfigPayload]:
        """Get configuration by hash."""
        pass


"""Run record model for CDB."""

from dataclasses import dataclass
from typing import Optional, Dict, Any


@dataclass
class RunRecord:
    """Represents a run record."""
    
    run_number: int
    data: Dict[str, Any]
    
    def __str__(self) -> str:
        return f"RunRecord(run={self.run_number})"


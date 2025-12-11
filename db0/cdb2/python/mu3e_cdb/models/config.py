"""Config payload model for CDB."""

from dataclasses import dataclass
from typing import Optional


@dataclass
class ConfigPayload:
    """Represents a configuration payload."""
    
    hash: str
    cfg_string: str
    
    def __str__(self) -> str:
        return f"ConfigPayload(hash={self.hash})"


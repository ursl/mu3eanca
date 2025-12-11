"""Payload model for CDB."""

from dataclasses import dataclass
from typing import Optional


@dataclass
class Payload:
    """Represents a CDB payload."""
    
    hash: str
    schema: str
    blob: str
    comment: str = ""
    
    def __str__(self) -> str:
        return f"Payload(hash={self.hash}, schema={self.schema})"


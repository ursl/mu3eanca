"""REST API database implementation."""

from typing import List, Dict, Optional
from .base import Database
from ..models.payload import Payload
from ..models.run_record import RunRecord
from ..models.config import ConfigPayload

# Lazy import of requests - only import when RestDatabase is actually used
_requests_available = False
try:
    import requests
    _requests_available = True
except ImportError:
    pass


class RestDatabase(Database):
    """REST API database implementation."""
    
    def __init__(self, uri: str, verbose: int = 0):
        super().__init__(uri, verbose)
        if not _requests_available:
            raise ImportError(
                "requests is required for RestDatabase. "
                "Install with: pip install requests"
            )
        self.name = "REST"
        self.base_url = uri.rstrip("/")
        self.session = requests.Session()
    
    def read_global_tags(self) -> List[str]:
        """Read all available global tags from database."""
        if not _requests_available:
            raise ImportError("requests module not available")
        try:
            response = self.session.get(f"{self.base_url}/globaltags")
            response.raise_for_status()
            return sorted(response.json())
        except Exception as e:  # requests.RequestException
            if self.verbose > 0:
                print(f"Error reading global tags: {e}")
            return []
    
    def read_tags(self, global_tag: str) -> List[str]:
        """Read all tags for a given global tag."""
        if not _requests_available:
            raise ImportError("requests module not available")
        try:
            response = self.session.get(f"{self.base_url}/globaltags/{global_tag}/tags")
            response.raise_for_status()
            return sorted(response.json())
        except Exception as e:
            if self.verbose > 0:
                print(f"Error reading tags: {e}")
            return []
    
    def read_iovs(self, tags: List[str]) -> Dict[str, List[int]]:
        """Read IOVs (Intervals of Validity) for given tags."""
        if not _requests_available:
            raise ImportError("requests module not available")
        iovs = {}
        for tag in tags:
            try:
                response = self.session.get(f"{self.base_url}/tags/{tag}/iovs")
                response.raise_for_status()
                iovs[tag] = sorted(response.json())
            except Exception as e:
                if self.verbose > 0:
                    print(f"Error reading IOVs for tag {tag}: {e}")
        return iovs
    
    def get_payload(self, hash: str) -> Optional[Payload]:
        """Get payload by hash."""
        if not _requests_available:
            raise ImportError("requests module not available")
        try:
            response = self.session.get(f"{self.base_url}/payloads/{hash}")
            response.raise_for_status()
            data = response.json()
            return Payload(
                hash=data.get("hash", hash),
                schema=data.get("schema", ""),
                blob=data.get("blob", ""),
                comment=data.get("comment", "")
            )
        except Exception:
            return None
    
    def get_run_record(self, run_number: int) -> Optional[RunRecord]:
        """Get run record for a given run number."""
        if not _requests_available:
            raise ImportError("requests module not available")
        try:
            response = self.session.get(f"{self.base_url}/runs/{run_number}")
            response.raise_for_status()
            data = response.json()
            return RunRecord(run_number=run_number, data=data)
        except Exception:
            return None
    
    def get_all_run_numbers(self, selection: str = "", detector: str = "") -> List[str]:
        """Get all run numbers, optionally filtered."""
        if not _requests_available:
            raise ImportError("requests module not available")
        try:
            params = {}
            if selection:
                params["class"] = selection
            if detector:
                params["detector"] = detector
            
            response = self.session.get(f"{self.base_url}/runs", params=params)
            response.raise_for_status()
            return sorted([str(r) for r in response.json()])
        except Exception as e:
            if self.verbose > 0:
                print(f"Error reading run numbers: {e}")
            return []
    
    def get_config(self, hash: str) -> Optional[ConfigPayload]:
        """Get configuration by hash."""
        if not _requests_available:
            raise ImportError("requests module not available")
        try:
            response = self.session.get(f"{self.base_url}/configs/{hash}")
            response.raise_for_status()
            data = response.json()
            return ConfigPayload(
                hash=data.get("hash", hash),
                cfg_string=data.get("cfgString", "")
            )
        except Exception:
            return None


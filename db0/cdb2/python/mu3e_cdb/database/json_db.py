"""JSON file-based database implementation."""

import json
import os
from pathlib import Path
from typing import List, Dict, Optional
from .base import Database
from ..models.payload import Payload
from ..models.run_record import RunRecord
from ..models.config import ConfigPayload


class JsonDatabase(Database):
    """JSON file-based database implementation."""
    
    def __init__(self, uri: str, verbose: int = 0):
        super().__init__(uri, verbose)
        self.name = "JSON"
        # Expand user home directory (~) and convert to absolute path
        expanded_uri = os.path.expanduser(uri)
        self.base_path = Path(expanded_uri).resolve()
        if not self.base_path.exists():
            raise ValueError(f"Database path does not exist: {uri} (resolved to: {self.base_path})")
    
    def read_global_tags(self) -> List[str]:
        """Read all available global tags from database."""
        gt_path = self.base_path / "globaltags"
        if not gt_path.exists():
            return []
        
        tags = []
        # Global tags are JSON files (not directories)
        for item in gt_path.iterdir():
            if item.is_file():
                tags.append(item.name)
        return sorted(tags)
    
    def read_tags(self, global_tag: str) -> List[str]:
        """Read all tags for a given global tag."""
        gt_file = self.base_path / "globaltags" / global_tag
        if not gt_file.exists():
            if self.verbose > 0:
                print(f"Global tag file not found: {gt_file}")
            return []
        
        try:
            with open(gt_file) as f:
                data = json.load(f)
                # Extract tags from JSON: {"gt": "...", "tags": [...]}
                tags = data.get("tags", [])
                return sorted(tags)
        except (json.JSONDecodeError, IOError) as e:
            if self.verbose > 0:
                print(f"Error reading global tag file {gt_file}: {e}")
            return []
    
    def read_iovs(self, tags: List[str]) -> Dict[str, List[int]]:
        """Read IOVs (Intervals of Validity) for given tags."""
        iovs = {}
        tags_dir = self.base_path / "tags"
        
        for tag in tags:
            tag_file = tags_dir / tag
            if not tag_file.exists():
                if self.verbose > 0:
                    print(f"Tag file not found: {tag_file}")
                continue
            
            try:
                with open(tag_file) as f:
                    data = json.load(f)
                    # Extract iovs from JSON: {"tag": "...", "iovs": [...]}
                    if "iovs" in data:
                        iovs[tag] = sorted(data["iovs"])
            except (json.JSONDecodeError, IOError) as e:
                if self.verbose > 0:
                    print(f"Error reading tag file {tag_file}: {e}")
        return iovs
    
    def get_payload(self, hash: str) -> Optional[Payload]:
        """Get payload by hash."""
        payloads_path = self.base_path / "payloads"
        payload_file = payloads_path / hash
        
        if not payload_file.exists():
            if self.verbose > 0:
                print(f"Payload file not found: {payload_file}")
            return None
        
        try:
            with open(payload_file) as f:
                data = json.load(f)
                # Try different possible field names for BLOB
                blob = data.get("BLOB") or data.get("blob") or data.get("BLOB64") or ""
                return Payload(
                    hash=data.get("hash", hash),
                    schema=data.get("schema", ""),
                    blob=blob,
                    comment=data.get("comment", "")
                )
        except (json.JSONDecodeError, IOError) as e:
            if self.verbose > 0:
                print(f"Error reading payload file {payload_file}: {e}")
            return None
    
    def get_run_record(self, run_number: int) -> Optional[RunRecord]:
        """Get run record for a given run number."""
        runs_path = self.base_path / "runs"
        run_file = runs_path / f"{run_number}.json"
        
        if not run_file.exists():
            return None
        
        with open(run_file) as f:
            data = json.load(f)
            return RunRecord(run_number=run_number, data=data)
    
    def get_all_run_numbers(self, selection: str = "", detector: str = "") -> List[str]:
        """Get all run numbers, optionally filtered."""
        runs_path = self.base_path / "runs"
        if not runs_path.exists():
            return []
        
        runs = []
        for run_file in runs_path.glob("*.json"):
            run_num = run_file.stem
            runs.append(run_num)
        
        return sorted(runs)
    
    def get_config(self, hash: str) -> Optional[ConfigPayload]:
        """Get configuration by hash."""
        configs_path = self.base_path / "configs"
        config_file = configs_path / hash
        
        if not config_file.exists():
            return None
        
        with open(config_file) as f:
            data = json.load(f)
            return ConfigPayload(
                hash=data.get("hash", hash),
                cfg_string=data.get("cfgString", "")
            )


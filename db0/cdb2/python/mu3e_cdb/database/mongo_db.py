"""MongoDB database implementation."""

from typing import List, Dict, Optional
from .base import Database
from ..models.payload import Payload
from ..models.run_record import RunRecord
from ..models.config import ConfigPayload


class MongoDatabase(Database):
    """MongoDB database implementation."""
    
    def __init__(self, uri: str, verbose: int = 0):
        super().__init__(uri, verbose)
        self.name = "MongoDB"
        try:
            from pymongo import MongoClient
        except ImportError:
            raise ImportError(
                "pymongo is required for MongoDB support. "
                "Install with: pip install pymongo"
            )
        self.client = MongoClient(uri)
        self.db = self.client.mu3e_cdb
    
    def read_global_tags(self) -> List[str]:
        """Read all available global tags from database."""
        collection = self.db.globaltags
        tags = collection.distinct("name")
        return sorted(tags)
    
    def read_tags(self, global_tag: str) -> List[str]:
        """Read all tags for a given global tag."""
        collection = self.db.tags
        tags = collection.find({"globalTag": global_tag}).distinct("name")
        return sorted(tags)
    
    def read_iovs(self, tags: List[str]) -> Dict[str, List[int]]:
        """Read IOVs (Intervals of Validity) for given tags."""
        collection = self.db.tags
        iovs = {}
        
        for tag in tags:
            doc = collection.find_one({"name": tag})
            if doc and "iovs" in doc:
                iovs[tag] = sorted(doc["iovs"])
        return iovs
    
    def get_payload(self, hash: str) -> Optional[Payload]:
        """Get payload by hash."""
        collection = self.db.payloads
        doc = collection.find_one({"hash": hash})
        
        if not doc:
            return None
        
        return Payload(
            hash=doc.get("hash", hash),
            schema=doc.get("schema", ""),
            blob=doc.get("blob", ""),
            comment=doc.get("comment", "")
        )
    
    def get_run_record(self, run_number: int) -> Optional[RunRecord]:
        """Get run record for a given run number."""
        collection = self.db.runs
        doc = collection.find_one({"runNumber": run_number})
        
        if not doc:
            return None
        
        # Convert MongoDB document to dict, excluding _id
        data = {k: v for k, v in doc.items() if k != "_id"}
        return RunRecord(run_number=run_number, data=data)
    
    def get_all_run_numbers(self, selection: str = "", detector: str = "") -> List[str]:
        """Get all run numbers, optionally filtered."""
        collection = self.db.runs
        query = {}
        
        if selection:
            query["class"] = {"$regex": selection, "$options": "i"}
        if detector:
            query["detector"] = {"$in": detector.split(",")}
        
        runs = collection.find(query).distinct("runNumber")
        return sorted([str(r) for r in runs])
    
    def get_config(self, hash: str) -> Optional[ConfigPayload]:
        """Get configuration by hash."""
        collection = self.db.configs
        doc = collection.find_one({"hash": hash})
        
        if not doc:
            return None
        
        return ConfigPayload(
            hash=doc.get("hash", hash),
            cfg_string=doc.get("cfgString", "")
        )


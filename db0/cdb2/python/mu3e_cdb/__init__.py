"""
Mu3e Conditions Database Python API

Main entry point for accessing Mu3e conditions data.
"""

from .conditions import Mu3eConditions
from .database.json_db import JsonDatabase

# Optional database backends
try:
    from .database.mongo_db import MongoDatabase
except ImportError:
    MongoDatabase = None

try:
    from .database.rest_db import RestDatabase
except ImportError:
    RestDatabase = None

__version__ = "0.1.0"
__all__ = [
    "Mu3eConditions",
    "JsonDatabase",
]

if MongoDatabase:
    __all__.append("MongoDatabase")
if RestDatabase:
    __all__.append("RestDatabase")


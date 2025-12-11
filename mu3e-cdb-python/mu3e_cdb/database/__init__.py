"""Database abstraction layer."""

from .base import Database
from .json_db import JsonDatabase

# Optional database backends - import only if available
try:
    from .mongo_db import MongoDatabase
except ImportError:
    MongoDatabase = None

try:
    from .rest_db import RestDatabase
except ImportError:
    RestDatabase = None

__all__ = ["Database", "JsonDatabase"]
if MongoDatabase:
    __all__.append("MongoDatabase")
if RestDatabase:
    __all__.append("RestDatabase")


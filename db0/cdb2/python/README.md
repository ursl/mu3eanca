# Mu3e Conditions Database Python API

Python API for accessing the Mu3e Conditions Database (CDB).

## Installation

```bash
cd db0/cdb2/python
pip install -e .

# For REST API support (optional):
pip install -e .[rest]

# For MongoDB support (optional):
pip install -e .[mongo]

# For all optional dependencies:
pip install -e .[all]
```

**Note**: The core package works with just the JSON database backend. REST and MongoDB backends are optional and only needed if you use those database types.

## Quick Start

```python
from mu3e_cdb import Mu3eConditions, JsonDatabase

# Initialize with JSON database
db = JsonDatabase("/path/to/json/db")
conditions = Mu3eConditions.instance("datav6.3=2025V0", db)

# Set run number
conditions.set_run_number(3265)

# Get pixel quality calibration
pixel_quality = conditions.get_calibration("pixelqualitylm_")
if pixel_quality:
    status = pixel_quality.get_status(chip_id=42, col=10, row=20)
    print(f"Pixel status: {status}")
    
    # Check if chip is dead
    is_dead = pixel_quality.is_chip_dead(chip_id=42)
    print(f"Chip 42 is dead: {is_dead}")

# Get pixel alignment
pixel_align = conditions.get_calibration("pixelalignment_")
if pixel_align:
    vx = pixel_align.get_vx(sensor_id=10)
    vy = pixel_align.get_vy(sensor_id=10)
    print(f"Sensor 10 position: ({vx}, {vy})")

# Get fibre quality
fibre_quality = conditions.get_calibration("fibrequality_")
if fibre_quality:
    lock = fibre_quality.get_asic_lock(asic_id=8)
    has_data = fibre_quality.get_asic_has_data(asic_id=8)
    print(f"ASIC 8: lock={lock}, has_data={has_data}")
```

## Database Backends

### JSON Database
```python
from mu3e_cdb import JsonDatabase

db = JsonDatabase("/path/to/json/db")
```

### MongoDB Database
```python
from mu3e_cdb import MongoDatabase

db = MongoDatabase("mongodb://localhost:27017")
```

### REST API Database
```python
from mu3e_cdb import RestDatabase

db = RestDatabase("http://localhost:3000/api")
```

## API Structure

- **Mu3eConditions**: Main entry point (singleton)
- **Database**: Abstract base for database implementations
- **Calibration**: Abstract base for calibration classes
- **Models**: Payload, RunRecord, ConfigPayload data models

## Calibration Classes

- `CalPixelQualityLM`: Pixel quality calibration with status enum (matches C++ calPixelQualityLM)
- `CalPixelAlignment`: Pixel alignment calibration (matches C++ calPixelAlignment)
- `CalFibreQuality`: Fibre quality calibration with status enum (matches C++ calFibreQuality)
- `CalFibreAlignment`: Fibre alignment calibration (matches C++ calFibreAlignment)
- `CalTileQuality`: Tile quality calibration with status enum (matches C++ calTileQuality)
- `CalTileAlignment`: Tile alignment calibration (matches C++ calTileAlignment)
- `CalMppcAlignment`: MPPC alignment calibration (matches C++ calMppcAlignment)

## Status Enumerations

Calibration classes use Python Enums for status values:

```python
from mu3e_cdb.calibrations.CalPixelQualityLM import PixelQualityStatus
from mu3e_cdb.calibrations.CalFibreQuality import FibreQualityStatus
from mu3e_cdb.calibrations.CalTileQuality import TileQualityStatus

# Pixel quality status
status = PixelQualityStatus.GOOD
print(PixelQualityStatus.to_string(status))  # "Good"
print(PixelQualityStatus.get_documentation())  # Documentation string

# Fibre quality status
fibre_status = FibreQualityStatus.GOOD

# Tile quality status
tile_status = TileQualityStatus.GOOD
```

## BLOB Format

The Python API fully implements BLOB encoding/decoding to match the C++ implementation:
- BLOBs are base64-encoded binary data
- All types (int, uint, double) are stored as 8-byte arrays
- Little-endian byte order
- Header magic number: 0xdeadface

## Running Tests

```bash
cd db0/cdb2/python
python -m pytest tests/
# or
python -m unittest discover tests
```


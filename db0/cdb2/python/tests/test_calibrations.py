"""Tests for calibration classes."""

import unittest
from mu3e_cdb.calibrations.CalPixelQualityLM import CalPixelQualityLM, PixelQualityStatus
from mu3e_cdb.calibrations.CalFibreQuality import CalFibreQuality, FibreQualityStatus
from mu3e_cdb.calibrations.CalTileQuality import CalTileQuality, TileQualityStatus


class TestCalibrations(unittest.TestCase):
    """Test calibration classes."""
    
    def test_pixel_quality_status(self):
        """Test PixelQualityStatus enum."""
        self.assertEqual(PixelQualityStatus.GOOD, 0)
        self.assertEqual(PixelQualityStatus.NOISY, 1)
        self.assertEqual(PixelQualityStatus.CHIP_NOT_FOUND, -1)
        
        doc = PixelQualityStatus.get_documentation()
        self.assertIn("Good", doc)
        self.assertIn("Noisy", doc)
    
    def test_fibre_quality_status(self):
        """Test FibreQualityStatus enum."""
        self.assertEqual(FibreQualityStatus.GOOD, 0)
        self.assertEqual(FibreQualityStatus.NOISY, 1)
    
    def test_tile_quality_status(self):
        """Test TileQualityStatus enum."""
        self.assertEqual(TileQualityStatus.GOOD, 0)
        self.assertEqual(TileQualityStatus.NOISY, 1)
    
    def test_pixel_quality_schema(self):
        """Test CalPixelQualityLM schema."""
        cal = CalPixelQualityLM()
        schema = cal.get_schema()
        self.assertIn("ui_id", schema)
        self.assertIn("ui_linkA", schema)
    
    def test_fibre_quality_schema(self):
        """Test CalFibreQuality schema."""
        cal = CalFibreQuality()
        schema = cal.get_schema()
        self.assertIn("ui_id", schema)
        self.assertIn("i_quality", schema)


if __name__ == '__main__':
    unittest.main()


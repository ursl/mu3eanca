"""Basic usage example for mu3e_cdb."""

from mu3e_cdb import Mu3eConditions, JsonDatabase
from mu3e_cdb.calibrations.CalPixelQualityLM import PixelQualityStatus


def main():
    # Initialize database and conditions
    # Replace with your actual database path
    # You can use ~ to refer to your home directory
    db = JsonDatabase("/data/project/mu3e/cdb")  # This is for merlin7. Replace with your actual path
    conditions = Mu3eConditions.instance("datav6.3=2025V0", db)
    
    # Enable verbose output to see what's happening
    conditions.set_verbosity(1)
    
    # Debug: Print available tags
    print(f"Global tag: {conditions.get_global_tag()}")
    tags = conditions.get_tags()
    print(f"Available tags ({len(tags)}): {tags[:10]}...")  # Print first 10 tags
    
    # Set run number
    conditions.set_run_number(3823)
    print(f"Set run number: {conditions.get_run_number()}")
    
    # Get pixel quality calibration
    print("\nTrying to get pixelqualitylm_ calibration...")
    try:
        pixel_quality = conditions.get_calibration("pixelqualitylm_")
        if pixel_quality:
            print(f"Successfully got pixel_quality calibration")
            print(f"  Hash: {pixel_quality.hash}")
            print(f"  Schema: {pixel_quality.get_schema()}")
            
            CHIP = 1313
            # Check pixel status
            status = pixel_quality.get_status(chip_id=CHIP, col=10, row=20)
            print(f"Pixel ({CHIP}, 10, 20) status: {status}")
            print(f"Status name: {PixelQualityStatus.to_string(status)}")
            
            # Check if chip is dead
            is_dead = pixel_quality.is_chip_dead(chip_id=CHIP)
            print(f"Chip {CHIP} is dead: {is_dead}")
            
            # Check link status
            is_link_bad = pixel_quality.is_link_bad(chip_id=CHIP, link=1)
            print(f"Chip {CHIP} link 1 is bad: {is_link_bad}")
            is_link_dead = pixel_quality.is_link_dead(chip_id=CHIP, link=1)
            print(f"Chip {CHIP} link 1 is dead: {is_link_dead}")
            
            # Get link status for all links
            print(f"Chip {CHIP} link status 0: {pixel_quality.get_link_status(chip_id=CHIP, link=0)}")
            print(f"Chip {CHIP} link status 1: {pixel_quality.get_link_status(chip_id=CHIP, link=1)}")
            print(f"Chip {CHIP} link status 2: {pixel_quality.get_link_status(chip_id=CHIP, link=2)}")
            
            # Get LVDS overflow rate
            overflow_rate = pixel_quality.get_lvds_overflow_rate(chip_id=CHIP)
            print(f"Chip {CHIP} LVDS overflow rate: {overflow_rate}")
        else:
            print(f"Failed to get pixel_quality calibration - returned None")
            print("Available tags containing 'pixel':")
            for tag in tags:
                if 'pixel' in tag.lower():
                    print(f"  - {tag}")
    except Exception as e:
        print(f"Error getting pixel_quality calibration: {e}")
        import traceback
        traceback.print_exc()
    
    # Get pixel alignment
    print("\nTrying to get pixelalignment_ calibration...")
    try:
        pixel_align = conditions.get_calibration("pixelalignment_")
        if pixel_align:
            print(f"Successfully got pixel_alignment calibration")
            vx = pixel_align.get_vx(sensor_id=1)
            vy = pixel_align.get_vy(sensor_id=1)
            vz = pixel_align.get_vz(sensor_id=1)
            print(f"Sensor 1 position: ({vx}, {vy}, {vz})")
            
            # Loop over all elements in the payload (like C++ iterator)
            print(f"\nIterating over all {len(pixel_align)} sensors in payload:")
            count = 0
            for sensor_id, const in pixel_align:
                vx = const.get("vx", 0.0)
                vy = const.get("vy", 0.0)
                vz = const.get("vz", 0.0)
                print(f"  Sensor {sensor_id}: v=({vx:.3f}, {vy:.3f}, {vz:.3f})")
                count += 1
                if count >= 10:  # Limit output for demo
                    print(f"  ... (showing first 10 of {len(pixel_align)} sensors)")
                    break
        else:
            print(f"Failed to get pixel_alignment calibration")
    except Exception as e:
        print(f"Error getting pixel_alignment calibration: {e}")
        import traceback
        traceback.print_exc()
    
    # Get fibre quality
    print("\nTrying to get fibrequality_ calibration...")
    try:
        fibre_quality = conditions.get_calibration("fibrequality_")
        if fibre_quality:
            print(f"Successfully got fibre_quality calibration")
            lock = fibre_quality.get_asic_lock(asic_id=8)
            has_data = fibre_quality.get_asic_has_data(asic_id=8)
            threshold = fibre_quality.get_asic_threshold(asic_id=8)
            print(f"ASIC 8: lock={lock}, has_data={has_data}, threshold={threshold}")
            
            # Example: Loop over all ASICs
            print(f"\nIterating over all {len(fibre_quality)} ASICs in payload:")
            count = 0
            for asic_id, const in fibre_quality:
                lock = const.get("lock", False)
                has_data = const.get("hasData", False)
                if lock or has_data:
                    threshold = const.get("thr", 0.0)
                    print(f"  ASIC {asic_id}: lock={lock}, has_data={has_data}, threshold={threshold}")
                    count += 1
                    if count >= 10:  # Limit output for demo
                        print(f"  ... (showing first 10 ASICs with data)")
                        break
        else:
            print(f"Failed to get fibre_quality calibration")
    except Exception as e:
        print(f"Error getting fibre_quality calibration: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()


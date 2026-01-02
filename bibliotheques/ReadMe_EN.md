# ESP32 Display Libraries Configuration

This repository contains pre-configured files for the libraries required for developing graphical interfaces on ESP32 with LVGL.

## File Structure

### 1. **ESP32_Display_Panel**
Library for managing displays and touchscreens on ESP32

#### Configuration Files:
- **esp_panel_drivers_conf.h**  
  Defines the hardware drivers used:
  - Display driver
  - Touch driver
  - Other display peripherals

- **esp_panel_board_supported_conf.h**  
  Specifies the hardware board used:  
  **WaveShare ESP32 S3 Touch 5B**

#### Location:
```
..\Arduino\libraries\ESP32_Display_Panel\
```

### 2. **LVGL (Light and Versatile Graphics Library)**
Embedded graphics library for creating user interfaces

#### Configuration File:
- **lv_conf.h**  
  Contains all LVGL configuration parameters:
  - Font selection
  - Gradient management
  - Memory configuration
  - Performance settings
  - Enabled/disabled features

#### Location:
```
..\Arduino\libraries\lvgl\
```

## Hardware Configuration

### Supported Board
- **WaveShare ESP32 S3 Touch 5B**
- Integrated touchscreen
- ESP32-S3 processor

## Installation

1. Ensure libraries are installed via Arduino Library Manager:
   - ESP32_Display_Panel
   - lvgl

2. Copy the configuration files provided in this repository to their respective locations

3. Restart Arduino IDE if necessary

## Customization

### To modify drivers:
Edit `esp_panel_drivers_conf.h` to:
- Change display driver
- Modify touch configuration
- Add/remove peripherals

### To change hardware board:
Modify `esp_panel_board_supported_conf.h` and select a new board from the supported options

### To adjust LVGL:
Adapt `lv_conf.h` according to your needs:
- Enable/disable gradients
- Change default fonts
- Adjust memory parameters
- Modify graphical features

## Important Notes

- These configurations are optimized for the **WaveShare ESP32 S3 Touch 5B**
- Paths are relative to your Arduino installation
- Backup your modifications before updating libraries
- Consult official documentation for advanced configurations

## Documentation

- [ESP32 Display Panel Library](https://github.com/espressif/esp-iot-solution)
- [LVGL Documentation](https://docs.lvgl.io/master/)
- [WaveShare ESP32 S3 Touch 5B](https://www.waveshare.com/wiki/ESP32-S3-Touch-5B)

## License

Configuration files are provided under MIT license.  
Refer to the respective library licenses for more information.
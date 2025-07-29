# ESP32 Lab Power Supply

DIY ESP32-based laboratory power supply with TFT display, rotary encoder, and OTA update.

## ✨ Features
- Dual output (voltage and current control)
- TFT display (TFT_eSPI library)
- Rotary encoder control
- OTA firmware update
- Presets: 3.3V, 5V, 12V
- INA219 current measurement

## 🛠 Hardware
- ESP32 DevKit
- ILI9488 TFT display
- INA219 current sensor
- Rotary encoder
- MOSFET or motorized potentiometer for control

## 📂 Setup
1. Install **ESP32 core** in Arduino IDE.
2. Install required libraries:
   - **TFT_eSPI** (for display)
   - **Adafruit INA219** (for current measurement)
   - **ArduinoOTA** (for OTA updates)
3. **Modify pin assignments in `User_Setup.h`** in the TFT_eSPI library folder to match your hardware configuration.
4. Set up your `User_Setup.h` for TFT.
5. Upload `LabPowerSupply.ino` to ESP32.

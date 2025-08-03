# ESP32 Lab Power Supply

DIY ESP32-based laboratory power supply with TFT display, rotary encoder, and OTA update.

##  Features
- Dual output (voltage and current control)
- TFT display (TFT_eSPI library)
- Rotary encoder control
- OTA firmware update
- Presets (3.3V, 5V, 7V) and customizable modes
- INA219 current measurement

##  Hardware
- ESP32 DevKit
- ILI9488 TFT display (3.5")
- SM-28BYJ-48-5V stepper motor x 2
- SM-ULN2003 stepper driver x 2
- INA219 current sensor x 2
- Rotary encoder
- LM2596S-PSUM module x 2
- SCT2650-STP-DWN module
- 40 x 40 12V fan

##  Setup
1. Install **ESP32 core** in Arduino IDE.
2. Install required libraries:
   - **TFT_eSPI** (for display)
   - **Adafruit INA219** (for current measurement)
   - **ArduinoOTA** (for OTA updates)
3. **Modify pin assignments in `User_Setup.h`** in the TFT_eSPI library folder to match your hardware configuration.
4. Set up your `User_Setup.h` for TFT.
5. Upload `LabPowerSupply.ino` to ESP32.

##  Recommended Pinout

Here is the recommended pin configuration for the ESP32 Lab Power Supply project. Please adjust the `User_Setup.h` file accordingly.

| Pin     | Function          | Description                          |
|---------|-------------------|--------------------------------------|
| GPIO 5  | Motor M1_IN1      | Motor driver input 1 for channel 1   |
| GPIO 19 | Motor M1_IN2      | Motor driver input 1 for channel 2   |
| GPIO 27 | Motor M1_IN3      | Motor driver input 1 for channel 3   |
| GPIO 33 | Motor M1_IN4      | Motor driver input 1 for channel 4   |
| GPIO 32 | Motor M2_IN1      | Motor driver input 2 for channel 1   |
| GPIO 25 | Motor M2_IN2      | Motor driver input 2 for channel 2   |
| GPIO 26 | Motor M2_IN3      | Motor driver input 2 for channel 3   |
| GPIO 14 | Motor M2_IN4      | Motor driver input 2 for channel 4   |
| GPIO 12 | Display LED       | TFT display LED pin                  |
| GPIO 15 | Display CS        | TFT display chip select              |
| GPIO 2  | Display DC        | TFT display data/command             |
| GPIO 4  | Display RST       | TFT display reset                    |
| GPIO 18 | Display CLK       | TFT display clock (SPI SCK)          |
| GPIO 23 | Display MOSI      | TFT display data line (SPI MOSI)     |
| GPIO 34 | Rotary Encoder A  | Rotary encoder signal A              |
| GPIO 35 | Rotary Encoder B  | Rotary encoder signal B              |
| GPIO 21 | INA219 SDA        | INA219 sensor data line (I2C)        |
| GPIO 22 | INA219 SCL        | INA219 sensor clock line (I2C)       |


> **Note:**  
> - This pinout is for the specific configuration used in this project.  
> - Modify the pins in the `User_Setup.h` file to fit your hardware setup.

### Hybrid Control System with Closed-Loop Feedback

This power supply uses a hybrid approach that combines **digital control** and **analog signal measurement**.  
The output voltage and current are continuously monitored through analog readings, while adjustments are performed digitally, resulting in more accurate control compared to systems that rely solely on digital signals.

The system operates with a **closed-loop feedback mechanism**, meaning it always uses the actual measured output values as the basis for regulation.  
This ensures stable and precise voltage and current control under varying load conditions.

### Open-Loop vs Closed-Loop Control

**Open-Loop (Without Feedback):**
Input Setpoint → [ Controller ] → Output  
(No real-time correction based on actual output)

**Closed-Loop (With Feedback):**
Input Setpoint → [ Controller ] → Output → [ Measurement ] → Feedback → [ Controller ]  
(The system continuously adjusts based on the actual measured values)

### Optional Bleeder Resistor for Smoother Voltage Adjustment

For more stable voltage adjustments, especially when fine-tuning the output voltage, it may be useful to add a bleeder resistor to the power supply circuit. This is optional and depends on your specific needs.

**Recommended Value:**
- **1 kΩ / 0.6 W** metal film resistor




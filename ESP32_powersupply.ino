#include <TFT_eSPI.h>
#include <SPI.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "Free_Fonts.h" 
#include <string>
#include <Adafruit_INA219.h>
#include <Stepper.h>
#include <ESPmDNS.h>.
#include <EEPROM.h>

#define EEPROM_ADDR_PSET 0 

Adafruit_INA219 ina1(0x41);  // Default title
Adafruit_INA219 ina2(0x40);  // Second module with a different title

const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

// Variables for button handling
int presetIndex = 0;
bool buttonPressed = false;
unsigned long buttonPressStartTime = 0;
bool longPressHandled = false;
const unsigned long longPressDuration = 500; // Long button press duration
bool longPressMode = false;
bool inCustomPresetMode = false;

// presets
float presetVoltage;
float psetCustom = 0; // Starting value

// Define character color variables
uint16_t BGcolor;
uint16_t StandardColor;
uint16_t Out1Volt;
uint16_t Out2Volt;
uint16_t Out1Amper;
uint16_t Out2Amper;
uint16_t Out1Watt;
uint16_t Out2Watt;
uint16_t PsetI;
uint16_t PsetA;

TFT_eSPI tft = TFT_eSPI();

#define CLK 34
#define DT 35
#define SW  13
#define TFT_LED_PIN 12 

#define STEPS_PER_REV 2048
// Motor 1
#define M1_IN1 5
#define M1_IN2 19
#define M1_IN3 27
#define M1_IN4 33

// Motor 2
#define M2_IN1 32
#define M2_IN2 25
#define M2_IN3 26
#define M2_IN4 14

Stepper stepper1(STEPS_PER_REV, M1_IN1, M1_IN3, M1_IN2, M1_IN4);
Stepper stepper2(STEPS_PER_REV, M2_IN1, M2_IN3, M2_IN2, M2_IN4);

float value_U1 = 5;
float value_U2 = 5;

float globalOutV1 = 5;
float globalOutV2 = 5;

ESP32Encoder encoder;

float value = 2.5;
const float minValue = 1.5;
const float maxValue = 20.0;
const float step = 0.1;

long LastEncoderCount = 0;

unsigned long lastEncoderCheck = 0;
const unsigned long encoderInterval = 200;  // ms

unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 500; //ms

//float lastDisplayedValue = -1;
int voltageState = 1;

// Position of text lines
  int ULine = 60;
  int ILine = 130;
  int PLine = 200;

  int xs1pos = 15;
  int xs2pos = 255;
  int yspos = 20;
  int borderW = 220;

// Position of the preset menu

  int PrXpos = 15;
  int PrYpos = 240;


void setup() {
  delay(1000);
  Wire.begin();  
  Wire.setClock(400000);  
  Serial.begin(115200);

  EEPROM.begin(512);  

   float value;
  EEPROM.get(EEPROM_ADDR_PSET, value);
  if (value < 1.5 || value > 20.0) value = 2.5;
  psetCustom = value;

  pinMode(TFT_LED_PIN, OUTPUT);
  digitalWrite(TFT_LED_PIN, HIGH); 

  tft.init();
  tft.setRotation(1);

  // OUTPUT warning
  tft.fillScreen(BGcolor);
  tft.setCursor(10, 30);
  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_YELLOW, BGcolor);
  tft.print("OUTPUT ACTIVE");
  tft.setCursor(10, 70);
  tft.print("WAIT FOR READY...");
  
  tft.setCursor(10, 110);   
  tft.setFreeFont(FM12);  
  tft.setTextColor(TFT_WHITE, BGcolor);  
  tft.print("Connection to WiFi");

  WiFi.begin(ssid, password);
  delay(500);

  unsigned long startAttemptTime = millis();  

int XDot = 10;
while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) { 
  delay(500);
  Serial.print(".");
  tft.setCursor(XDot, 140);
  XDot += 12;
  tft.print(".");
}

if (WiFi.status() == WL_CONNECTED) {
 
  tft.setCursor(10, 170);
  tft.print("WiFi connected!");
  delay(2000);
  setupOTA();  
} else {
  
  tft.setCursor(10, 170);
  tft.print("WiFi connection failed!");
  tft.setCursor(10, 200);
  tft.print("Continuing without OTA...");
  delay(2000);
}
    
  tft.setFreeFont(FM12);
if (!ina1.begin()) {
  
  tft.setCursor(10, 240);
  tft.setTextColor(TFT_RED); 
  tft.print("INA219 #1 not detected!");
  delay(2000);
  while (1);
} else {
  
  tft.setCursor(10, 240);
  tft.setTextColor(TFT_GREEN); 
  tft.print("INA219 #1 detected.");
  delay(2000);
}
ina1.setCalibration_32V_2A();

if (!ina2.begin()) {
 
  tft.setCursor(10, 280);
  tft.setTextColor(TFT_RED);
  tft.print("INA219 #2 not detected!");
  delay(2000);
  while (1);
} else {
  tft.setCursor(10, 280);
  tft.setTextColor(TFT_GREEN);
  tft.print("INA219 #2 detected.");
  delay(2000);
}
ina2.setCalibration_32V_2A();
  
  stepper1.setSpeed(8);
  stepper2.setSpeed(8);

   
  pinMode(SW, INPUT_PULLUP);

  encoder.attachHalfQuad(DT, CLK);
      
    // Color definitions
 
BGcolor = tft.color565(0, 0, 0);
StandardColor = tft.color565(232, 228, 227);
Out1Volt= tft.color565(65, 227, 20);
Out2Volt= tft.color565(65, 227, 20);
Out1Amper= tft.color565(158, 230, 226);
Out2Amper= tft.color565(158, 230, 226);
Out1Watt= tft.color565(237, 161, 232);
Out2Watt= tft.color565(237, 161, 232);
PsetI= tft.color565(171, 168, 167);
PsetA= tft.color565(217, 23, 9);


// Font definitions

#define StandardFont &FreeMonoBold18pt7b
#define measText &FreeMonoBold12pt7b

  tft.fillScreen(BGcolor);
  
  // Menu structure setup

  tft.setCursor(xs1pos, 10);   
  tft.setFreeFont(FM9);  
  tft.setTextColor(TFT_WHITE, BGcolor);  
  tft.print("CH1");
  tft.setCursor(xs2pos, 10);
  tft.print("CH2");
   
  // Left and right frame
  tft.drawRect(xs1pos, yspos, borderW, 200, TFT_GREEN);
  tft.drawRect(xs2pos, yspos, borderW, 200, TFT_WHITE);

 // Preset menu frame
   tft.drawRect(PrXpos, PrYpos, 460, 60, TFT_WHITE);

 // Preset texts
  tft.setCursor(PrXpos + 5, PrYpos +35);   
  tft.setFreeFont(FSS9);  
  tft.setTextColor(PsetI, BGcolor);  
  tft.print("Pre1: 3.3V   ");
  tft.print("Pre2: 5.0V   ");
  tft.print("Pre3: 7.0V   ");
  tft.print("Custom:");
  char bufferSet[20];
  sprintf(bufferSet, "%.2f V", psetCustom);
  tft.setCursor(PrXpos + 375, PrYpos + 35);   
  tft.setFreeFont(FSS9);  
  tft.setTextColor(TFT_WHITE, BGcolor);  
  tft.print(bufferSet);

  
voltageState = 1;
while (abs(ina1.getBusVoltage_V() - globalOutV1) > 0.05) {
  regulateOutputVoltageCH1();
  delay(2); 
}
drawVoltage(value_U1);

voltageState = 2;
while (abs(ina2.getBusVoltage_V() - globalOutV2) > 0.05) {
  regulateOutputVoltageCH2();
  delay(2); 
}
drawVoltage(value_U2);

encoder.setCount((int)(value_U1 / step) * 2);
voltageState = 1;

    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_WHITE, BGcolor);
    tft.setCursor(xs1pos+30, ULine + 30);
    tft.print("MEAS: [               ]");
    tft.setCursor(xs2pos+30, ULine + 30);
    tft.print("MEAS: [               ]");

}

void setupOTA() {
  ArduinoOTA.setHostname("esp32-tap");
  static std::string Oldpercent = "";

  ArduinoOTA.onStart([]() {
    tft.fillScreen(BGcolor); 
    tft.setFreeFont(FF21);
    tft.setTextColor(StandardColor, BGcolor);
    tft.setCursor(10, 30);
    tft.print("OTA: update starting...");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char buffer[32];
    tft.setCursor(10, 80);
    tft.setFreeFont(FF18);
    tft.print("Progress :");
    sprintf(buffer, "%3u %%", (progress * 100) / total);
    tft.setCursor(105, 80);
    tft.setTextColor(TFT_BLACK, BGcolor);
    tft.print(Oldpercent.c_str());
    tft.setCursor(105, 80);
    tft.setTextColor(TFT_LIGHTGREY, BGcolor);
    tft.print(buffer);

    Oldpercent = std::string(buffer);
  });

  ArduinoOTA.onEnd([]() {
    tft.fillRect(0, 90, 480, 40, BGcolor);
    tft.setCursor(10, 120);
    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_GREEN, BGcolor);
    tft.print("Update complete.");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    tft.fillRect(0, 90, 480, 60, BGcolor);
    tft.setCursor(10, 120);
    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_RED, BGcolor);
    tft.print("OTA hiba: ");
    switch (error) {
      case OTA_AUTH_ERROR: tft.print("Auth"); break;
      case OTA_BEGIN_ERROR: tft.print("Begin"); break;
      case OTA_CONNECT_ERROR: tft.print("Connect"); break;
      case OTA_RECEIVE_ERROR: tft.print("Receive"); break;
      case OTA_END_ERROR: tft.print("End"); break;
      default: tft.print("Unknown"); break;
    }
  });

  ArduinoOTA.begin();
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastMeasurementTime >= measurementInterval) {
    lastMeasurementTime = currentTime;
    drawMeasuredVoltage(); 
    drawAmper();
    drawWatt();
  }

 

    // Only handle the encoder and short press in normal mode
  if (!longPressMode) {

    // Select active voltage variable (with reference)
    float& activeValue = (voltageState == 1) ? value_U1 : value_U2;

   // Get raw encoder value (interpreted as step count)
    int encSteps = encoder.getCount() / 2;
    float newValue = encSteps * step;

    // Handle boundaries
    if (newValue < minValue) {
        newValue = minValue;
        encoder.setCount((int)(minValue / step) * 2);
    } else if (newValue > maxValue) {
        newValue = maxValue;
        encoder.setCount((int)(maxValue / step) * 2);
    }

    activeValue = newValue;
    drawVoltage(activeValue);
}

 unsigned long currentMillis = millis();
  if (currentMillis - lastEncoderCheck >= encoderInterval) {
    lastEncoderCheck = currentMillis;

    
    int buttonState = digitalRead(SW);

    if (buttonState == LOW) {
      if (!buttonPressed) {
        buttonPressed = true;
        buttonPressStartTime = currentMillis;
        longPressHandled = false;
      } else {
        if (!longPressHandled && (currentMillis - buttonPressStartTime >= longPressDuration)) {
          longPressHandled = true;
          longPressMode = !longPressMode; 
          if (longPressMode) {
             inCustomPresetMode = false;
             presetIndex = 0;  
            applyPreset(presetIndex);
            drawPresetMenu(presetIndex);

          } else {
            exitLongPressMode();

          }
        }
      }
    } else {
      if (buttonPressed) {
        if (!longPressHandled && !longPressMode) {
          voltageState = (voltageState == 1) ? 2 : 1;
          float encoderValue = (voltageState == 1) ? value_U1 : value_U2;
          encoder.setCount((int)(encoderValue / step) * 2);

      if (voltageState == 1) {
        // Change frame colors to the active channel
        tft.drawRect(xs1pos, yspos, borderW, 200, TFT_GREEN);  
        tft.drawRect(xs2pos, yspos, borderW, 200, TFT_WHITE);  
      } else {
        
        tft.drawRect(xs1pos, yspos, borderW, 200, TFT_WHITE);  
        tft.drawRect(xs2pos, yspos, borderW, 200, TFT_GREEN);  
      }


        }
        buttonPressed = false;
      }
    }
  }

  if (longPressMode) {
  if (inCustomPresetMode) {
    handleCustomPreset(); 
  } else {
    handleLongPressMode(); 
  }
}

  regulateOutputVoltageCH1();
  regulateOutputVoltageCH2();
  ArduinoOTA.handle();
}


void enterLongPressMode() {
  
  // Entered long press mode
 
  tft.drawRect(PrXpos, PrYpos, 460, 60, TFT_GREEN);
  presetIndex = 0;
  applyPreset(presetIndex);
  
}

void exitLongPressMode() {
  encoder.attachHalfQuad(DT, CLK);
  encoder.setCount(LastEncoderCount * 2);   
  // Exited long press mode
  // Redraw preset bar with inactive color
  tft.setCursor(PrXpos + 5, PrYpos + 35);   
  tft.setFreeFont(FSS9);  

  tft.setTextColor(PsetI, BGcolor);  
  tft.print("Pre1: 3.3V   ");

  tft.setTextColor(PsetI, BGcolor);  
  tft.print("Pre2: 5.0V   ");

  tft.setTextColor(PsetI, BGcolor);  
  tft.print("Pre3: 7.0V   ");

  tft.setTextColor(PsetI, BGcolor);  
  tft.print("Custom:");

  tft.drawRect(PrXpos, PrYpos, 460, 60, TFT_WHITE);
  if (voltageState == 1){
    globalOutV1 = presetVoltage;
  tft.drawRect(xs2pos, yspos, borderW, 200, TFT_WHITE); 
  tft.drawRect(xs1pos, yspos, borderW, 200, TFT_GREEN); 
   
  }else{
     globalOutV2 = presetVoltage;
  tft.drawRect(xs1pos, yspos, borderW, 200, TFT_WHITE); 
  tft.drawRect(xs2pos, yspos, borderW, 200, TFT_GREEN); 
   
   }
  drawVoltage(presetVoltage);
  
  }

void handleLongPressMode() {
  
  if (inCustomPresetMode) {
    handleCustomPreset();
    return;  
  }

  int buttonState = digitalRead(SW);
  encoder.detach(); 
  if (buttonState == LOW) {
  if (!buttonPressed) {
    buttonPressed = true;
    buttonPressStartTime = millis();
    longPressHandled = false;
  }
} else {
  if (buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressStartTime;
    if (pressDuration < longPressDuration) {
      // Switch preset on button press

      presetIndex = (presetIndex + 1) % 4;

      if (presetIndex == 3) {
        inCustomPresetMode = true;
        drawPresetMenu(3);
      } else {
        inCustomPresetMode = false;
        applyPreset(presetIndex);
        drawPresetMenu(presetIndex);
      }
    }
    buttonPressed = false;
  }
}
}

void drawPresetMenu(int activeIndex) {
  tft.setCursor(PrXpos + 5, PrYpos + 35);
  tft.setFreeFont(FSS9);

  for (int i = 0; i < 4; i++) {
    if (i == activeIndex) {
      tft.setTextColor(PsetA, BGcolor);  // Active color
    } else {
      tft.setTextColor(PsetI, BGcolor);  // Inactive color
    }

    switch (i) {
      case 0:
        tft.print("Pre1: 3.3V   ");
        break;
      case 1:
        tft.print("Pre2: 5.0V   ");
        break;
      case 2:
        tft.print("Pre3: 7.0V   ");
        break;
      case 3:
        tft.print("Custom:");
        break;
    }
  }
}

void applyPreset(int index) {
       encoder.detach(); 
  switch (index) {
    case 0:
      presetVoltage = 3.3;
      LastEncoderCount = 33;
     break;
    case 1:
      presetVoltage = 5.0;
      LastEncoderCount = 50;
      break;
    case 2:
      presetVoltage = 7.0;
      LastEncoderCount = 70;
      break;
    default:
      
      presetVoltage = psetCustom; 
      LastEncoderCount = psetCustom * 10;
      break;  
  }

 // Set the voltage value to the corresponding channel
  if (voltageState == 1) {
    globalOutV1 = presetVoltage;
  } else {
    globalOutV2 = presetVoltage;
  }
  drawVoltage(presetVoltage);
}

void handleCustomPreset() {
  encoder.attachHalfQuad(DT, CLK);
  static long lastCount = 0;
  static bool initialized = false;
  static std::string OldpsetCustom = "";

  if (!initialized) {
  initialized = true;

  // psetCustom retrieved from EEPROM has already been set previously

  long rawCount = (psetCustom) / 0.1;
  //Limits of adjustment with the encoder.

  if (rawCount < 15) rawCount = 15; // 15 = 1.5V
  else if (rawCount > 200) rawCount = 200; // 200 = 20V

  encoder.setCount(rawCount  - 2); 
  lastCount = rawCount;

  if (voltageState == 1) {
    globalOutV1 = psetCustom;
  } else {
    globalOutV2 = psetCustom;
  }

  
  char bufferSet[20];
  sprintf(bufferSet, "%.2f V", psetCustom);
  tft.setCursor(PrXpos + 375, PrYpos + 35);   
  tft.setFreeFont(FSS9);  
  tft.setTextColor(TFT_WHITE, BGcolor);  
  tft.print(bufferSet);
  OldpsetCustom = std::string(bufferSet);

  drawVoltage(psetCustom);
}
   //Limits of adjustment with the encoder.

  long rawCount = encoder.getCount() / 2;

   if (rawCount < 15) {  // 15 = 1.5V
    rawCount = 15;
    encoder.setCount(15 * 2);
  } else if (rawCount > 200) {  // 200 = 20V
    rawCount = 200;
    encoder.setCount(200  * 2);
  }

  long currentCount = rawCount;

   if (currentCount != lastCount) {
    lastCount = currentCount;
    psetCustom = 0 + currentCount * 0.1;

     if (voltageState == 1) {
      globalOutV1 = psetCustom;
    } else {
      globalOutV2 = psetCustom;
    }

    char bufferSet[20];
    sprintf(bufferSet, "%.2f V", psetCustom);

    if (OldpsetCustom != std::string(bufferSet)) {
      tft.setCursor(PrXpos + 375, PrYpos + 35);   
      tft.setFreeFont(FSS9);  
      tft.setTextColor(BGcolor, BGcolor);  
      tft.print(OldpsetCustom.c_str());

      tft.setCursor(PrXpos + 375, PrYpos + 35);   
      tft.setFreeFont(FSS9);  
      tft.setTextColor(TFT_WHITE, BGcolor);  
      tft.print(bufferSet);

      OldpsetCustom = std::string(bufferSet);
    }
    
    drawVoltage(psetCustom);
    
  }

  int buttonState = digitalRead(SW);
  
  if (buttonState == LOW) {
    float lastSavedValue;
    EEPROM.get(EEPROM_ADDR_PSET, lastSavedValue);
    

    if (!isnan(psetCustom) && !isnan(lastSavedValue)) {
      if (fabs(lastSavedValue - psetCustom) > 0.05) {
        EEPROM.put(EEPROM_ADDR_PSET, psetCustom);
        EEPROM.commit();
        LastEncoderCount = psetCustom * 10;
        
      }
    }

    inCustomPresetMode = false;  
    
    presetIndex = -1;
    applyPreset(presetIndex);  
    initialized = false;
  }
}



void drawMeasuredVoltage() {
  static std::string OldRealV1 = "";
  static std::string OldRealV2 = "";

  float realV1 = ina1.getBusVoltage_V(); // U1
  float realV2 = ina2.getBusVoltage_V(); //U2

  char bufferSet[20];

  const float epsilon = 0.01;  
// --- U1 measured ---

  sprintf(bufferSet,  "%.2f V ", realV1);
  if (fabs(realV1 - atof(OldRealV1.c_str())) > epsilon) {  
    
    tft.setCursor(xs1pos+105, ULine + 30);
    tft.setFreeFont(FF21);
    tft.setTextColor(BGcolor, BGcolor);
    tft.print(OldRealV1.c_str());

    // New value

    tft.setCursor(xs1pos+105, ULine + 30);
    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_WHITE, BGcolor);  
    tft.print(bufferSet);

    OldRealV1 = std::string(bufferSet);
  }

 // --- U2 measured ---
  
  sprintf(bufferSet, "%.2f V ", realV2);
  if (fabs(realV2 - atof(OldRealV2.c_str())) > epsilon) {  
    
    tft.setCursor(xs2pos+105, ULine + 30);
    tft.setFreeFont(FF21);
    tft.setTextColor(BGcolor, BGcolor);
    tft.print(OldRealV2.c_str());

   // New value
    
    tft.setCursor(xs2pos+105, ULine + 30);
    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_WHITE, BGcolor);
    tft.print(bufferSet);

    OldRealV2 = std::string(bufferSet);
  }
}


void drawVoltage(float voltage) {
  static std::string OldVoltage1 = "";
  static std::string OldVoltage2 = "";
  char bufferSet[10];
  sprintf(bufferSet, "%.1f", voltage);

  if (voltageState == 1) {
    if (OldVoltage1 != std::string(bufferSet)) {
     
      tft.setCursor(xs1pos + 80, ULine);
      tft.setFreeFont(StandardFont);
      tft.setTextColor(BGcolor, BGcolor);
      tft.print(OldVoltage1.c_str());
      tft.setFreeFont(measText);
      tft.print(" V");

      // Draw new value

      tft.setCursor(xs1pos + 6, ULine);
      tft.setFreeFont(StandardFont);
      tft.setTextColor(StandardColor, BGcolor);
      tft.print("U1: ");
      tft.setCursor(xs1pos + 80, ULine);
      tft.setTextColor(Out1Volt, BGcolor);
      tft.print(bufferSet);
      tft.setFreeFont(measText);
      tft.print(" V");

      OldVoltage1 = std::string(bufferSet);
      globalOutV1 = voltage;
    }
  } else {
    if (OldVoltage2 != std::string(bufferSet)) {
     
      tft.setCursor(xs2pos + 80, ULine);
      tft.setFreeFont(StandardFont);
      tft.setTextColor(BGcolor, BGcolor);
      tft.print(OldVoltage2.c_str());
      tft.setFreeFont(measText);
      tft.print(" V");

      // Draw new value

      tft.setCursor(xs2pos + 6, ULine);
      tft.setFreeFont(StandardFont);
      tft.setTextColor(StandardColor, BGcolor);
      tft.print("U2: ");
      tft.setCursor(xs2pos + 80, ULine);
      tft.setTextColor(Out2Volt, BGcolor);
      tft.print(bufferSet);
      tft.setFreeFont(measText);
      tft.print(" V");

      OldVoltage2 = std::string(bufferSet);
      globalOutV2 = voltage;
    }
  }
}


/* This code performs feedback control for regulating the output voltages across the channels.
 It reads the measured voltage, applies a moving average filter to smooth the signal, 
 and compares it with the target voltage. If the difference exceeds the specified tolerance, 
 the stepper motor is adjusted to bring the voltage closer to the target.
The `stepInterval` ensures that adjustments happen at fixed intervals, preventing excessive motor movements.
*/
//==============================================================================
void regulateOutputVoltageCH1() {
  static unsigned long lastStepTime = 0;
  static float voltageFiltered1 = 0;
  float readVoltage = ina1.getBusVoltage_V();
  float targetVoltage = globalOutV1;

  // Filtering: moving average style (0.9 weight = slow, stable)
  voltageFiltered1 = 0.5 * voltageFiltered1 + 0.5 * readVoltage;

  // Set the tolerance value for voltage difference. If the measured voltage differs from the target by more than this value, adjustment is made.
  // A smaller tolerance results in finer control and more frequent adjustments.
  const float tolerance = 0.03; 
  // Defines the time interval (in milliseconds) between each step. 
  // The stepper motor will only step if this interval has passed since the last step.
  const unsigned long stepInterval = 10; 

  if (millis() - lastStepTime > stepInterval) {
    float diff = voltageFiltered1 - targetVoltage;

    if (abs(diff) > tolerance) {
      int direction = (diff > 0) ? 1 : -1;
      stepper1.step(direction);
    }

    lastStepTime = millis();
  }
}


void regulateOutputVoltageCH2() {
  static unsigned long lastStepTime = 0;
  static float voltageFiltered2 = 0;
  float readVoltage = ina2.getBusVoltage_V();
  float targetVoltage = globalOutV2;

  // Filtering: moving average style (0.9 weight = slow, stable)
  voltageFiltered2 = 0.5 * voltageFiltered2 + 0.5 * readVoltage;

  // Set the tolerance value for voltage difference. If the measured voltage differs from the target by more than this value, adjustment is made.
  // A smaller tolerance results in finer control and more frequent adjustments.
  const float tolerance = 0.03; 
  // Defines the time interval (in milliseconds) between each step. 
  // The stepper motor will only step if this interval has passed since the last step.
  const unsigned long stepInterval = 10; 

  if (millis() - lastStepTime > stepInterval) {
    float diff = voltageFiltered2 - targetVoltage;

    if (abs(diff) > tolerance) {
      int direction = (diff > 0) ? 1 : -1;
      stepper2.step(direction);
  
    }

    lastStepTime = millis();
  }
}

// =====================================================================


void drawAmper() {
  static std::string OldAmper1 = "";
  static std::string OldAmper2 = "";
  static std::string OldmA1 = "";
  static std::string OldmA2 = "";

static float maxAmper1_mA = 0;
static float maxAmper2_mA = 0;
static std::string OldMaxmA1 = "";
static std::string OldMaxmA2 = "";

unsigned long lastChangeTime = 0;
float lastCurrent1 = 0;

  float realAmper1 = ina1.getCurrent_mA() / 1000.0;
  float realAmper2 = ina2.getCurrent_mA() / 1000.0;

  if (fabs(realAmper1) < 0.005) realAmper1 = 0.0;
  if (fabs(realAmper2) < 0.005) realAmper2 = 0.0;

  
float current1_mA = realAmper1 * 1000;
if (current1_mA > maxAmper1_mA) maxAmper1_mA = current1_mA;

float current2_mA = realAmper2 * 1000;
if (current2_mA > maxAmper2_mA) maxAmper2_mA = current2_mA;

if (realAmper1 == 0.0) {
  maxAmper1_mA = 0;
}
if (realAmper2 == 0.0) {
  maxAmper2_mA = 0;
}

 
  char bufferA[10];
  char buffermA[10];

  // === I1 ===
  sprintf(bufferA, "%.2f", realAmper1);
  sprintf(buffermA, "[ %.0f", realAmper1 * 1000);  

  if (OldAmper1 != std::string(bufferA) || OldmA1 != std::string(buffermA)) {
    // --- Delete ---
    // Clear top line (A)
    tft.setCursor(xs1pos + 80, ILine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(BGcolor, BGcolor);
    tft.print(OldAmper1.c_str());
    tft.setFreeFont(measText);
    tft.print(" A");
    

    // Clear bottom line (mA)
    tft.setCursor(xs1pos + 85, ILine + 25);  
    tft.setFreeFont(FF21);
    tft.setTextColor(BGcolor, BGcolor);
    tft.print(OldmA1.c_str());
    tft.print(" mA ]");

    // --- New value ---
    // Top line (A)
    tft.setCursor(xs1pos + 6, ILine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(StandardColor, BGcolor);
    tft.print("I1: ");
    tft.setCursor(xs1pos + 80, ILine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(Out1Amper, BGcolor);
    tft.print(bufferA);
    tft.setFreeFont(measText);
    tft.print(" A");

    char bufferMaxmA[10];

if (maxAmper1_mA == 0){
 sprintf(bufferMaxmA, "    0     ");  
}else{ 
 sprintf(bufferMaxmA, "< %.0f", maxAmper1_mA); 
}

if (OldMaxmA1 != std::string(bufferMaxmA)) {
  tft.setCursor(xs1pos + 10, ILine + 25); 
  tft.setFreeFont(FF21);
  tft.setTextColor(BGcolor, BGcolor); 
  tft.print(OldMaxmA1.c_str());

  tft.setCursor(xs1pos + 10, ILine + 25); 
  tft.setTextColor(TFT_YELLOW, BGcolor);
  tft.print(bufferMaxmA);

  OldMaxmA1 = std::string(bufferMaxmA);
}
    tft.setCursor(xs1pos + 85, ILine + 25);  
    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_WHITE, BGcolor);
    tft.print(buffermA);
    tft.print(" mA ]");

    OldAmper1 = std::string(bufferA);
    OldmA1 = std::string(buffermA);
  }

  // === I2 ===
  sprintf(bufferA, "%.2f", realAmper2);
  sprintf(buffermA, "[ %.0f", realAmper2 * 1000);

  if (OldAmper2 != std::string(bufferA) || OldmA2 != std::string(buffermA)) {
    
    tft.setCursor(xs2pos + 80, ILine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(BGcolor, BGcolor);
    tft.print(OldAmper2.c_str());
    tft.setFreeFont(measText);
    tft.print(" A");

    tft.setCursor(xs2pos + 80, ILine + 25);
    tft.setFreeFont(FF21);
    tft.setTextColor(BGcolor, BGcolor);
    tft.print(OldmA2.c_str());
    tft.print(" mA ]");

    
    tft.setCursor(xs2pos + 6, ILine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(StandardColor, BGcolor);
    tft.print("I2: ");
    tft.setCursor(xs2pos + 80, ILine);
    tft.setTextColor(Out2Amper, BGcolor);
    tft.setFreeFont(StandardFont);
    tft.print(bufferA);
    tft.setFreeFont(measText);
    tft.print(" A");

    char bufferMaxmA[10];

if (maxAmper2_mA == 0){
 sprintf(bufferMaxmA, "    0     ");  
}else{ 
 sprintf(bufferMaxmA, "< %.0f", maxAmper2_mA); 
}

if (OldMaxmA2 != std::string(bufferMaxmA)) {
  tft.setCursor(xs2pos + 10, ILine + 25);  
  tft.setFreeFont(FF21);
  tft.setTextColor(BGcolor, BGcolor);  
  tft.print(OldMaxmA2.c_str());

  tft.setCursor(xs2pos + 10, ILine + 25);  
  tft.setTextColor(TFT_YELLOW, BGcolor);
  tft.print(bufferMaxmA);

  OldMaxmA2 = std::string(bufferMaxmA);
}


    tft.setCursor(xs2pos + 80, ILine + 25);
    tft.setFreeFont(FF21);
    tft.setTextColor(TFT_WHITE, BGcolor);
    tft.print(buffermA);
    tft.print(" mA ]");

    OldAmper2 = std::string(bufferA);
    OldmA2 = std::string(buffermA);
  }
}



void drawWatt() {
  static float oldWatt1 = -1.0;  
  static float oldWatt2 = -1.0;  

  // Actual values from the INA sensors
  float realVolt1 = ina1.getBusVoltage_V();
  float realAmper1 = ina1.getCurrent_mA() / 1000.0;
  if (fabs(realAmper1) < 0.005) realAmper1 = 0.0;
  float realWatt1 = realVolt1 * realAmper1;

  float realVolt2 = ina2.getBusVoltage_V();
  float realAmper2 = ina2.getCurrent_mA() / 1000.0;
  if (fabs(realAmper2) < 0.005) realAmper2 = 0.0;
  float realWatt2 = realVolt2 * realAmper2;

  char bufferSet[10];

  // --- P1 ---
  if (oldWatt1 == -1.0 || fabs(realWatt1 - oldWatt1) >= 0.01) {  
    sprintf(bufferSet, "%.2f", realWatt1);
   
    tft.setCursor(xs1pos+80, PLine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(BGcolor, BGcolor); 
    tft.print(String(oldWatt1, 2)); 
    tft.setFreeFont(measText);
    tft.print(" W");

    tft.setCursor(xs1pos+6, PLine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(StandardColor, BGcolor);
    tft.print("P1: ");
    tft.setCursor(xs1pos+80, PLine);
    tft.setTextColor(Out1Watt, BGcolor);
    tft.print(bufferSet); 
    tft.setFreeFont(measText);
    tft.print(" W");

    oldWatt1 = realWatt1; 
  }

  // --- P2 ---
  if (oldWatt2 == -1.0 || fabs(realWatt2 - oldWatt2) >= 0.01) {  
    sprintf(bufferSet, "%.2f", realWatt2);

    tft.setCursor(xs2pos+80, PLine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(BGcolor, BGcolor); 
    tft.print(String(oldWatt2, 2)); 
    tft.setFreeFont(measText);
    tft.print(" W");

    tft.setCursor(xs2pos+6, PLine);
    tft.setFreeFont(StandardFont);
    tft.setTextColor(StandardColor, BGcolor);
    tft.print("P2: ");
    tft.setCursor(xs2pos+80, PLine);
    tft.setTextColor(Out2Watt, BGcolor);
    tft.print(bufferSet); 
    tft.setFreeFont(measText);
    tft.print(" W");

    oldWatt2 = realWatt2; 
  }
}

// =====================================================================
//  MOTION SWORD (ESP32 + MPU6050, acting as a Bluetooth mouse)
// =====================================================================
//  Swing the sword hard and your laptop gets a left click, which is
//  "attack" in Minecraft.
//
//  Before uploading:
//    1. Install the ESP32 boards (Boards Manager: "esp32 by Espressif").
//    2. Install the "ESP32 BLE Mouse" library by T-vK as a ZIP:
//       https://github.com/T-vK/ESP32-BLE-Mouse
//    3. Tools > Board > "ESP32 Dev Module", and pick your USB port.
//
//  Wiring:
//    MPU6050 VCC -> 3V3     MPU6050 SDA -> D21
//    MPU6050 GND -> GND     MPU6050 SCL -> D22
//
//  After uploading, pair "Minecraft Sword" in your laptop's Bluetooth
//  settings. Open the Serial Monitor at 115200 baud to see each swing.
// =====================================================================

#include <Wire.h>      // I2C: how the ESP32 talks to the MPU6050
#include <BleMouse.h>  // makes the ESP32 show up as a Bluetooth mouse

// Bluetooth name, manufacturer, and battery level (%) shown on the laptop
BleMouse bleMouse("Minecraft Sword", "ESP32", 100);

// I2C address of the MPU6050. It's 0x68 unless the board's AD0 pin is
// connected to 3.3V, in which case use 0x69.
const int MPU_ADDR = 0x68; 

// How hard you have to swing. The sensor gives raw numbers from about
// -32768 to 32767, and 16384 = 1g (the pull of gravity) at its default
// setting. So:
//   - Lower this if you have to swing too hard.
//   - Raise it if it clicks when you're not swinging.
// Gravity alone can read up to 16384 on the X axis, so mount the sensor so
// X isn't pointing straight down while you hold the sword.
const float swingThreshold = 15000.0; 

// Remembers that we already clicked for this swing, so one swing = one hit
bool alreadySwung = false;

void setup() {
  Serial.begin(115200);  // match this number in the Serial Monitor
  
  // Start advertising as a Bluetooth mouse so the laptop can pair with it
  bleMouse.begin();
  
  // Start I2C on pins 21 (SDA) and 22 (SCL)
  Wire.begin(21, 22); 
  
  // The MPU6050 starts in sleep mode. Writing 0 to its power management
  // register (0x6B) wakes it up. Talking to the sensor directly like this
  // (instead of using the Adafruit library) also works with clone GY-521
  // boards that the library refuses to detect.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  
  Serial.println("Sword is fully armed! Waiting for Bluetooth connection...");
}

void loop() {
  // Do nothing until the laptop has paired and connected
  if(bleMouse.isConnected()) {
    
    // Ask the sensor for its acceleration readings. They're stored in six
    // registers starting at 0x3B: X high byte, X low byte, then Y, then Z.
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);  // false = keep the bus open for the read
    Wire.requestFrom(MPU_ADDR, 6, true); 
    
    // Combine each high byte and low byte into one number per axis
    int16_t AcX = Wire.read()<<8 | Wire.read(); 
    int16_t AcY = Wire.read()<<8 | Wire.read(); 
    int16_t AcZ = Wire.read()<<8 | Wire.read(); 

    // Only the X axis is used for swings. abs() means a swing in either
    // direction counts. AcY and AcZ are read but not used (yet).
    float currentSwingForce = abs(AcX);

    // Big enough force, and we haven't clicked for this swing yet: attack!
    if (currentSwingForce > swingThreshold && !alreadySwung) {
      
      bleMouse.click(MOUSE_LEFT);
      Serial.println("SWING DETECTED! Left click sent.");
      
      alreadySwung = true; 
      delay(300); // 300ms cooldown so a single swing doesn't click 5 times
      
    } else if (currentSwingForce < (swingThreshold - 3000)) {
      // The force has dropped well below the threshold, so the swing is
      // over and the next one can click again. The 3000 gap stops a
      // reading that hovers right at the threshold from clicking repeatedly.
      alreadySwung = false;
    }
  }
  delay(20);  // read about 50 times per second
}

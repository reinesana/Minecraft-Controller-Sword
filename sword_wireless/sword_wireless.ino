#include <Wire.h>
#include <BleMouse.h>

BleMouse bleMouse("Minecraft Sword", "ESP32", 100);

const int MPU_ADDR = 0x68; 
// Raw accelerometer data goes up to ~32768. Lower this number if you have to swing too hard!
const float swingThreshold = 15000.0; 
bool alreadySwung = false;

void setup() {
  Serial.begin(115200);
  
  // Start Bluetooth
  bleMouse.begin();
  
  // Start I2C on pins 21 (SDA) and 22 (SCL)
  Wire.begin(21, 22); 
  
  // Wake up the MPU6050 sensor
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Power management register
  Wire.write(0);    // Write 0 to wake it up
  Wire.endTransmission(true);
  
  Serial.println("Sword is fully armed! Waiting for Bluetooth connection...");
}

void loop() {
  if(bleMouse.isConnected()) {
    
    // Ask the sensor for the current physical acceleration
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // Starting register for X, Y, Z data
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true); 
    
    // Read the raw physical force
    int16_t AcX = Wire.read()<<8 | Wire.read(); 
    int16_t AcY = Wire.read()<<8 | Wire.read(); 
    int16_t AcZ = Wire.read()<<8 | Wire.read(); 

    // Calculate the force hitting the X-axis
    float currentSwingForce = abs(AcX);

    // Trigger a left click if the physical force crosses the threshold
    if (currentSwingForce > swingThreshold && !alreadySwung) {
      
      bleMouse.click(MOUSE_LEFT);
      Serial.println("SWING DETECTED! Left click sent.");
      
      alreadySwung = true; 
      delay(300); // 300ms cooldown so a single swing doesn't click 5 times
      
    } else if (currentSwingForce < (swingThreshold - 3000)) {
      alreadySwung = false; // Reset the trigger when the swing stops
    }
  }
  delay(20);
}
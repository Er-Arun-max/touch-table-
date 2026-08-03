#include <Wire.h>
#include <FastLED.h>

// =========================================================
// 📄 DIRECT REGISTER DEFINITIONS
// =========================================================
#define MHD_R       0x2B
#define NHD_R       0x2C
#define NCL_R       0x2D
#define FDL_R       0x2E
#define MHD_F       0x2F
#define NHD_F       0x30
#define NCL_F       0x31
#define FDL_F       0x32
#define ELE0_T      0x41
#define ELE0_R      0x42
#define ELE1_T      0x43
#define ELE1_R      0x44
#define ELE2_T      0x45
#define ELE2_R      0x46
#define ELE3_T      0x47
#define ELE3_R      0x48
#define ELE4_T      0x49
#define ELE4_R      0x4A
#define ELE5_T      0x4B
#define ELE5_R      0x4C
#define FIL_CFG     0x5D
#define ELE_CFG     0x5E

#define TOU_THRESH  0x06
#define REL_THRESH  0x0A

// I2C Addresses for the two boards
#define MPR121_ADDR_1  0x5A  // ADD pin connected to GND
#define MPR121_ADDR_2  0x5B  // ADD pin connected to 3.3V

// =========================================================
// ⚙️ PIN & LIGHTING STRUCTURE
// =========================================================
int irqpin = 2;             // Tied IRQ lines from both boards on D2
#define LED_DATA_PIN    3   // FastLED data line connected to D3

#define NUM_ZONES       12  // 6 zones from Board 1 + 6 zones from Board 2
#define LEDS_PER_ZONE   10   // 1 LED per zone
#define TOTAL_LEDS      120  // 12 total physical pixels

CRGB leds[TOTAL_LEDS];

// Persistent states across all 12 zones
int activeLedsCount[NUM_ZONES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool isZoneTouched[NUM_ZONES]  = {false, false, false, false, false, false, false, false, false, false, false, false};

uint8_t rainbowStartHue = 0; 

// Function Declarations
void mpr121_setup(uint8_t address);
void set_register(int address, unsigned char r, unsigned char v);
void read_mpr121(uint8_t address, uint8_t zoneOffset);

void setup() {
  pinMode(irqpin, INPUT_PULLUP);
  
  Serial.begin(9600);
  Wire.begin();

  Serial.println("--- DUAL MPR121 (6 Electrodes Each) + FASTLED ---");

  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, TOTAL_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000); 
  
  FastLED.clear();
  FastLED.show();

  // Configure Board 1 (0x5A) and Board 2 (0x5B)
  mpr121_setup(MPR121_ADDR_1);
  mpr121_setup(MPR121_ADDR_2);
}

void loop() {
  // 1. READ SENSOR DATA FROM BOTH CHIPS WHEN INTERRUPT FIRES
  if (digitalRead(irqpin) == LOW) {
    // Read lower 6 zones (ELE0..ELE5) from Board #1 -> Zones 0..5
    read_mpr121(MPR121_ADDR_1, 0);
    
    // Read upper 6 zones (ELE0..ELE5) from Board #2 -> Zones 6..11
    read_mpr121(MPR121_ADDR_2, 6);
  }

  // 2. ANIMATION ENGINE RUNS EVERY FRAME
  for (int zone = 0; zone < NUM_ZONES; zone++) {
    if (isZoneTouched[zone]) {
      if (activeLedsCount[zone] < LEDS_PER_ZONE) {
        activeLedsCount[zone]++;
      }
    } else {
      if (activeLedsCount[zone] > 0) {
        activeLedsCount[zone]--;
      }
    }
  }

  // 3. RENDER ENVIRONMENT FRAME BUFFER
  FastLED.clear();

  for (int zone = 0; zone < NUM_ZONES; zone++) {
    int zoneStartIndex = zone * LEDS_PER_ZONE;

    for (int i = 0; i < activeLedsCount[zone]; i++) {
      uint8_t pixelHue = rainbowStartHue + (zone * 20) + (i * 10);
      leds[zoneStartIndex + i] = CHSV(pixelHue, 255, 255);
    }
  }

  rainbowStartHue += 2; 
  FastLED.show();
  
  delay(40); // Physics / animation speed
}

// Helper function to read touch states from a specific MPR121 address
void read_mpr121(uint8_t address, uint8_t zoneOffset) {
  Wire.requestFrom((int)address, 2); 
  if (Wire.available() >= 2) {
    byte LSB = Wire.read();
    byte MSB = Wire.read();
    uint16_t touched = ((MSB << 8) | LSB); 

    // Extract ELE0 through ELE5 from this board
    for (int electrode = 0; electrode < 6; electrode++) {
      isZoneTouched[zoneOffset + electrode] = touched & (1 << electrode);
    }
  }
}

// =========================================================
// 🛠️ CHIP CONFIGURATION SEQUENCER (Parameterized by Address)
// =========================================================
void mpr121_setup(uint8_t address) {
  // Put chip into Stop Mode before changing settings
  set_register(address, ELE_CFG, 0x00); 
  
  // Section A: Rising Edge Baseline Filtering
  set_register(address, MHD_R, 0x01);
  set_register(address, NHD_R, 0x01);
  set_register(address, NCL_R, 0x0E);
  set_register(address, FDL_R, 0x00);

  // Section B: Falling Edge Baseline Filtering
  set_register(address, MHD_F, 0x01);
  set_register(address, NHD_F, 0x05);
  set_register(address, NCL_F, 0x01);
  set_register(address, FDL_F, 0x00);
  
  // Section C: Sensitivity Thresholds for ELE0 through ELE5
  set_register(address, ELE0_T, TOU_THRESH); set_register(address, ELE0_R, REL_THRESH);
  set_register(address, ELE1_T, TOU_THRESH); set_register(address, ELE1_R, REL_THRESH);
  set_register(address, ELE2_T, TOU_THRESH); set_register(address, ELE2_R, REL_THRESH);
  set_register(address, ELE3_T, TOU_THRESH); set_register(address, ELE3_R, REL_THRESH);
  set_register(address, ELE4_T, TOU_THRESH); set_register(address, ELE4_R, REL_THRESH);
  set_register(address, ELE5_T, TOU_THRESH); set_register(address, ELE5_R, REL_THRESH);
  
  // Section D: Filter Configuration
  set_register(address, FIL_CFG, 0x24);
  
  // Section F: Global Charge Discharge Current Configuration (CDC) - Max current boost
  set_register(address, 0x5C, 0x3F); 
  
  // Section G: Charge Time Configuration (CDT)
  set_register(address, 0x5B, 0x20); 

  // Section E: Electrode Configuration Selector
  // 0x86 = Baseline tracking enabled + 6 active electrodes (ELE0 through ELE5)
  set_register(address, ELE_CFG, 0x86); 
}

void set_register(int address, unsigned char r, unsigned char v) {
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}
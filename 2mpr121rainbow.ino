#include <Wire.h>
#include <FastLED.h>

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

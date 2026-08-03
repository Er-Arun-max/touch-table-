CRGB leds[TOTAL_LEDS];

// Persistent states across all 12 zones
int activeLedsCount[NUM_ZONES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool isZoneTouched[NUM_ZONES]  = {false, false, false, false, false, false, false, false, false, false, false, false};

uint8_t rainbowStartHue = 0; 

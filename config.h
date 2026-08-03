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

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

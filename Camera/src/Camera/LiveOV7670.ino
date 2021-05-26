




// change setup.h to switch between buffered and pixel-by-pixel processing
#include "setup.h"



void setup() {
  
  CLKPR = 0x80; // enter clock rate change mode
  CLKPR = 0; // set prescaler to 0. WAVGAT MCU has it 3 by default.

  initializeScreenAndCamera();
}


void loop() {
  processFrame();
}

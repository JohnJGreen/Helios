
#include "CMLib.h" 
EnergyMonitor cm1;

void setup()
{    
  Serial.begin(9600); 
  cm1.current(1, 55); // Current: input pin, calibration.
}

void loop()
{
  double Irms = cm1.calcIrms(1450); // Clac Irms: num of reads - an offset

  if(Irms<0.04){ //readings under 0.04 is assumed noise
    Irms = 0;
  }
  Serial.println(Irms*120.0); // current x voltage
}


#include "CMLib.h"


#include "Arduino.h"



//--------------------------------------------------------------------------------------
// Sets the pins to be used for voltage and current sensors
//--------------------------------------------------------------------------------------

void EnergyMonitor::current(unsigned int _inPinI, double _ICAL)
{
   inPinI = _inPinI;
   ICAL = _ICAL;
   offsetI = ADC_COUNTS>>1;
}


double EnergyMonitor::calcIrms(unsigned int Number_of_Samples)
{
  
  
	int SupplyVoltage=5000;
   

  
  for (unsigned int n = 0; n < Number_of_Samples; n++)
  {
    sampleI = analogRead(inPinI);

    // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset, 
	//  then subtract this - signal is now centered on 0 counts.
    offsetI = (offsetI + (sampleI-offsetI)/1024);
	filteredI = sampleI - offsetI;

    // Root-mean-square method current
    // 1) square current values
    sqI = filteredI * filteredI;
    // 2) sum 
    sumI += sqI;
  }

  double I_RATIO = ICAL *((SupplyVoltage/1000.0) / (ADC_COUNTS));
  Irms = I_RATIO * sqrt(sumI / Number_of_Samples); 

  //Reset accumulators
  sumI = 0;
//--------------------------------------------------------------------------------------       
 
  return Irms;
}


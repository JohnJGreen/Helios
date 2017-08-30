
#ifndef CMLib_h
#define CMLib_h

#include "Arduino.h"

#define ADC_BITS    10

#define ADC_COUNTS  (1<<ADC_BITS)


class EnergyMonitor
{
  public:

    void current(unsigned int _inPinI, double _ICAL);

    double calcIrms(unsigned int NUMBER_OF_SAMPLES);

    double Irms;

  private:

    //Set Voltage and current input pins
    unsigned int inPinI;
    //Calibration coefficients
    double ICAL;

    //--------------------------------------------------------------------------------------
    // Variable declaration for emon_calc procedure
    //--------------------------------------------------------------------------------------
	int sampleI;                     

	double filteredI;                  
	double offsetI;                          //Low-pass filter output               

	double sqI,sumI;              //sq = squared, sum = Sum, inst = instantaneous

};

#endif

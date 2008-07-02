#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <fstream>
#include <iostream>
#include "defines.h"

/**********************************************************************
* samples holds the three temperatures and corresponding three 		  *
* resistances for the user's input. temps are lowest to highest		  *
* resistances are corresponding values and will be highest to 		  *
* lowest due to the nature of thermistors.							  *
**********************************************************************/
struct samples {
  double t1, t2, t3, r1, r2, r3;
};

typedef enum 
{
	CLT=0xabc, 
	IAT 
}SensorType;

struct inc_entry {
  int ms_val;
  int adc;
  int temp_f;
  int temp_c;
  int ohms;
};

class thermistor {
    
  public:
    thermistor(char scale, samples &input, int bias, SensorType type, char *cmnt);	// Parameterized constructor
    thermistor(thermistor &);									// Copy constructor
    int write_inc_file();
    void modify_s19_file(char filename[]);
    ~thermistor(void);
    
  protected:
    char temp_scale;		// C for Centigrade, F for Fahrenheit
    double CA, CB, CC;		// Steinhart-Hart coefficients
    int bias_value;		// iat bias resistor value in ohms
    char *comment;		// User comments about sensor
    inc_entry entries[256];	// array to hold all entries to be written to inc file
    SensorType sensor;		// is this IAT (0) or CLT (1)?
    
    // set_coefficients takes the temp-resistance input samples and 
    // sets the coefficients for this thermistor
    void set_coefficients(samples &in);

    // t_of_r takes a resistance and returns the temp in Kelvins
     int t_of_r(int r);
};
#endif

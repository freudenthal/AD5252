/*
AD5252.h Library for communicating with AD5252 Digital Potentiometer
Last update 9/3/2015
Sean Kirkpatrick
*/

#ifndef AD5252_h	//check for multiple inclusions
#define AD5252_h

#include "Arduino.h"
#include "i2c_t3.h"

#define I2CTimeout 1000

enum class AD5252commandMode{NOP,RestoreEEMEMtoRDAC,StoreRDACtoEEMEM,Decrement6dB,DecrementAll6dB,DecrementOneStep,DecrementAllOneStep,ResetEEMEMtoAllRDAC,Increment6dB,IncrementAll6dB,IncrementOneStep,IncrementAllOneStep};

class AD5252
{
	public:
		AD5252();
		~AD5252();
		bool isConnected();
		//calibration information
		void getTolerances();
		float getRDACTolerance(uint8_t Channel);
		uint8_t getRDACValue(uint8_t Channel);
		float getRDACValueFloat(uint8_t Channel);
		//increment and decrement commands
		void decrementRDAC6dB(uint8_t Channel);
		void incrementRDAC6dB(uint8_t Channel);
		void decrementRDACOneStep(uint8_t Channel);
		void incrementRDACOneStep(uint8_t Channel);
		void decrementAllRDAC6dB();
		void incrementAllRDAC6dB();
		void decrementAllRDACOneStep();
		void incrementAllRDACOneStep();
		//set to specific values
		void setRDACValueInt(uint8_t Channel, uint8_t Target);
		void setRDACValueFloat(uint8_t Channel, float Target);

	private:
		uint8_t Address;
		void setRDACValue(uint8_t Channel, AD5252commandMode CommandSetting);
		void SetCommandByte(uint8_t Command);
		void SendI2CCommand();
		uint8_t singleByteI2CRead();
		uint8_t CommandByte;
		uint8_t MSBByte;
		float Rab1Tolerance;	//RDAC1 tolerance (%)
		float Rab3Tolerance;	//RDAC3 tolerance (%)
		float Rab1;				//calculated absolute end-to-end resistance for RDAC1
		float Rab3;				//calculated absolute end-to-end resistance for RDAC3
		void SendI2C();
		void SendI2CCommandThreeBytes();
};
#endif
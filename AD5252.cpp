/*
AD5252.cpp Library for communicating with AD5252 Digital Potentiometer
Last update 9/3/2015
Sean Kirkpatrick
*/

#include "AD5252.h"
#define combine(high,low) ( ( (uint16_t)(high << 8) ) | (uint16_t)(low) )
#define lowbyte(value) ( (uint8_t)(value) )
#define highbyte(value) ( (uint8_t)(value>>8) )
const float Rab = 1000.0;	//end-to-end resistance, default 1000 Ohm

AD5252::AD5252()
{
	Address = 0x2C;
	MSBByte = 0x00;
	
	// Serial.println(Rab1);
	// Serial.println(Rab3);
}

AD5252::~AD5252()
{
}

bool AD5252::isConnected()
{
	int Status = 5;
	Wire.beginTransmission(Address);
    Status = Wire.endTransmission(I2C_STOP);
    if (Status == 0)
    {
    	return true;
    }
    else
    {
    	return false;
    }
}

void AD5252::SetCommandByte(uint8_t Command)
{
	CommandByte = Command;
}

void AD5252::getTolerances()
{
	Rab1Tolerance = getRDACTolerance(0);
	Rab3Tolerance = getRDACTolerance(1);
	Rab1 = Rab*(1+ (Rab1Tolerance/100));
	Rab3 = Rab*(1+ (Rab3Tolerance/100));
}

float AD5252::getRDACTolerance(uint8_t Channel)
//Tolerance is a percentage represented by two bytes.  a signed int8 for the integer followed by unsigned int for the decimal
//Channel should be an integer 0 or 1 for RDAC1 and RDAC3 selection.
{
	uint8_t Add1;	//integer value address
	uint8_t Add2;	//decimal value address
	uint8_t integerValue;
	uint8_t decimalValue;
	Channel = constrain(Channel,0,1); // 0 <= Channel <= 1
	if(Channel == 1)
	{
		Add1 = B00111110;
		Add2 = B00111111;
	}
	else	//default is RDAC1 values
	{
		Add1 = B00111010;
		Add2 = B00111011;
	}
	SetCommandByte(Add1);
	SendI2CCommand();
	integerValue = singleByteI2CRead();
	SetCommandByte(Add2);
	SendI2CCommand();
	decimalValue = singleByteI2CRead();
	return (float)((int8_t)integerValue)+(float)decimalValue*pow(2,-8);
}

uint8_t AD5252::getRDACValue(uint8_t Channel)
{
	Channel = constrain(Channel, 0, 1);
	switch(Channel)
	{
		case 1:
			SetCommandByte(0x03);	//RDAC3
			break;
		case 0:
			SetCommandByte(0x01);	//RDAC1
			break;
		default:
			break;
	}
	SendI2CCommand(); //"Dummy" write to set address for read request
	return singleByteI2CRead();
}

float AD5252::getRDACValueFloat(uint8_t Channel)
{
	getTolerances();
	uint8_t RDACintvalue = getRDACValue(Channel);
	Channel = constrain(Channel, 0, 1);
	switch(Channel)
	{
		case 0:
			return RDACintvalue*Rab1/256 + 75.0;
		case 1:
			return RDACintvalue*Rab3/256 + 75.0;
		default:
			return 0.0;
	}
}

void AD5252::decrementRDAC6dB(uint8_t Channel)
{
	setRDACValue(Channel, AD5252commandMode::Decrement6dB);
}

void AD5252::incrementRDAC6dB(uint8_t Channel)
{
	setRDACValue(Channel, AD5252commandMode::Increment6dB);
}

void AD5252::decrementRDACOneStep(uint8_t Channel)
{
	setRDACValue(Channel, AD5252commandMode::DecrementOneStep);
}

void AD5252::incrementRDACOneStep(uint8_t Channel)
{
	setRDACValue(Channel, AD5252commandMode::IncrementOneStep);
}

void AD5252::decrementAllRDAC6dB()
{
	setRDACValue(0, AD5252commandMode::DecrementAll6dB);	//channels ignored for "all" commands
}

void AD5252::incrementAllRDAC6dB()
{
	setRDACValue(0, AD5252commandMode::IncrementAll6dB);	//channels ignored for "all" commands
}

void AD5252::decrementAllRDACOneStep()
{
	setRDACValue(0, AD5252commandMode::DecrementAllOneStep);	//channels ignored for "all" commands
}

void AD5252::incrementAllRDACOneStep()
{
	setRDACValue(0, AD5252commandMode::IncrementAllOneStep);	//channels ignored for "all" commands
}

void AD5252::setRDACValueFloat(uint8_t Channel, float Target)
//set RDAC on Channel to closest step equivalent to Target (Ohms)
{

}

void AD5252::setRDACValueInt(uint8_t Channel, uint8_t Target)
{
	uint8_t _commandbyte = B00000000;
	Channel = constrain(Channel, 0, 1);
	switch(Channel)
	{
		case 0:	//RDAC1
			_commandbyte = _commandbyte | B00000001;
			break;
		case 1: //RDAC3
			_commandbyte = _commandbyte | B00000011;
			break;
		default:
			break;
	}
	MSBByte = Target;
	SetCommandByte(_commandbyte);
	SendI2CCommandThreeBytes();
}

void AD5252::setRDACValue(uint8_t Channel, AD5252commandMode Command)
{
	uint8_t _commandbyte;
	switch(Command)
	{
		case AD5252commandMode::Decrement6dB:
			_commandbyte = B10011000;
			break;
		case AD5252commandMode::DecrementAll6dB:
			_commandbyte = B10100000;
			break;		
		case AD5252commandMode::DecrementOneStep:
			_commandbyte = B10101000;
			break;
		case AD5252commandMode::DecrementAllOneStep:
			_commandbyte = B10110000;
			break;
		case AD5252commandMode::Increment6dB:
			_commandbyte = B11000000;
			break;
		case AD5252commandMode::IncrementAll6dB:
			_commandbyte = B11001000;
			break;
		case AD5252commandMode::IncrementOneStep:
			_commandbyte = B11010000;
			break;
		case AD5252commandMode::IncrementAllOneStep:
			_commandbyte = B11011000;
			break;
		default:
			_commandbyte = B10000000;	//no operation
	}
	Channel = constrain(Channel, 0, 1);
	switch(Channel)
	{
		case 1:
			_commandbyte = _commandbyte | B00000011;
			break;
		case 0:
			_commandbyte = _commandbyte | B00000001;
			break;
		default:
			break;
	}
	SetCommandByte(_commandbyte);
	SendI2CCommand();
}

uint8_t AD5252::singleByteI2CRead()
{
	Wire.requestFrom(Address, 1, I2C_STOP, I2CTimeout*2);
    if (Wire.available())
    {
	    return Wire.readByte();
	}
	else
	{
		return 0x00;
	}
}

void AD5252::SendI2CCommand()
//two byte command call
{
  bool MoveOn = false;
  const int MaxAttempts = 16;
  int CurrentAttempt = 0;
  int SendSuccess = 5;
  while (!MoveOn)
  {
	Wire.beginTransmission(Address);
	Wire.write(CommandByte);
	SendSuccess = Wire.endTransmission(I2C_STOP,I2CTimeout);
    if(SendSuccess != 0)
    {
      Wire.finish();
      Wire.resetBus();
      CurrentAttempt++;
      if (CurrentAttempt > MaxAttempts)
      {
        MoveOn = true;
        Serial.println("Unrecoverable I2C transmission error with AD5252.");
      }
    }
    else
    {
    	MoveOn = true;
    }
  }
}

void AD5252::SendI2CCommandThreeBytes()
//three byte command call, used for setting RDACs to specific values
{
  bool MoveOn = false;
  const int MaxAttempts = 16;
  int CurrentAttempt = 0;
  int SendSuccess = 5;
  while (!MoveOn)
  {
	Wire.beginTransmission(Address);
	Wire.write(CommandByte);
	Wire.write(MSBByte);
	SendSuccess = Wire.endTransmission(I2C_STOP,I2CTimeout);
    if(SendSuccess != 0)
    {
      Wire.finish();
      Wire.resetBus();
      CurrentAttempt++;
      if (CurrentAttempt > MaxAttempts)
      {
        MoveOn = true;
        Serial.println("Unrecoverable I2C transmission error with AD5252.");
      }
    }
    else
    {
    	MoveOn = true;
    }
  }
}
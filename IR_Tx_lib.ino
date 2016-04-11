/* IR Transmitter program.

  This is example code to demo use of the IR transmit protocol. This code runs with the 
    
    To Improve on: 
  - Make rising edge immediate! (like Josh). Use a pin register define instead of digitalWrite/Mode etc
    
    IR PROTOCOL DESCRIPTION:
    - Transmits using Manchester encoding
      # each bit period is 0.8ms but this includes 2 sub-bits of the Manchester code:  
      
              1                        0
      --------                          --------
              |                        |  
              |                        |
              |                        |
               --------        --------
     <-----0.8ms------>        <-----0.8ms------>          
                 
    We send a start bit which is a little different for channel 1 or 2:
    - Channel 1: 200us on, 600us off
    - Channel 2: 600us on, 200us off 
 
 */ 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#####################################################################################################################################################################
//-----Constants-------------------------------------------------------------------------------------------------------------------------------------------------------
const int           BIT_PERIOD              = 800;                                      // I don't recommend changing this!
const unsigned int  SIGNAL_PERIOD           = 40000;                                    

//-----Pin Defines------------------------------------------
#define     IR_Pin      10                                                              // IR output pin. Different pins can be used, you just need to set up the appropiate timer.                           

//-----Functions--------------------------------------------
char Transmit(unsigned long Data, char Bits, int Bit_Period, long Signal_Period, char Channel);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#####################################################################################################################################################################
//-----Program---------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  // Start Serial port
  Serial.begin(115200);  
  Serial.println("Power On");
    
  // Configure Pins
  pinMode(IR_Pin, INPUT); 
  
  // Configure timer to output 38KHz
  ICR1 = 210;                                                                           // 38KHz - Freq = Clk / (2 * (ICR1 + 1)
  TCCR1A = 0b00010000;
  TCCR1B = 0b00011001; 
}

void loop()
{
  static unsigned long i;
  
  while(!Transmit(i, 17, BIT_PERIOD, SIGNAL_PERIOD, 1))  {}

  i -= (1 << (32-17));    
}

//------------------------------------------------------------------------------------
char Transmit(unsigned long Data, char Bits, int Bit_Period, long Signal_Period, char Channel)
{
  // Data is sent MSB first, and a max of 32 bits of data can be sent. This excludes a start bit. The function returns zero if the Signal_Period
  // hasn't elapsed before the next call. For Irregular transmissions, put SIGNAL_PERIOD = 0 and call transmit when you need.

  static unsigned long Time_at_Previous_Transmit = 0;
  if(micros() - Time_at_Previous_Transmit < Signal_Period)  {
    return(0);
  }
  Time_at_Previous_Transmit += Signal_Period;                                        

  char index;  
  int Half_Bit_Period = Bit_Period / 2;   
  unsigned long Wait_Until;

  // Start half bit - 200us high for channel 1, 600us high for channel 2
  pinMode(IR_Pin, OUTPUT);
  
  if(Channel == 1)  {
    Wait_Until += Half_Bit_Period - 200; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}     
    pinMode(IR_Pin, INPUT);
    Wait_Until += Half_Bit_Period + 200; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}
  }
  else  {
    Wait_Until += Half_Bit_Period + 200; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}     
    pinMode(IR_Pin, INPUT);
    Wait_Until += Half_Bit_Period - 200; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}    
  }
    
  // Next send the data:
  for(index = 0; index < Bits; index++)
  {
    if(!bitRead(Data, (31 - index)))
      {
        pinMode(IR_Pin, OUTPUT);
        Wait_Until += Half_Bit_Period; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}
        pinMode(IR_Pin, INPUT);
        Wait_Until += Half_Bit_Period; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}          
      }       
    else
      {
        pinMode(IR_Pin, INPUT);
        Wait_Until += Half_Bit_Period; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}
        pinMode(IR_Pin, OUTPUT);
        Wait_Until += Half_Bit_Period; while(micros() - Time_at_Previous_Transmit < Wait_Until) {}   
      }      
  }

  pinMode(IR_Pin, INPUT);
  return(1);
}




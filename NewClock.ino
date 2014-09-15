// Include required libraries
#include <Wire.h>

// Set constants
#define LEDMAX 24

// Set variables
boolean POWER = false;
int SWITCH = 2;
int TIME_SECONDS = 0,
  TIME_MINUTES = 0,
  TIME_HOURS = 0, // 24H format
  TIME_DAY = 0, // 0 = Sunday
  TIME_DATE = 1,
  TIME_MONTH = 1,
  TIME_YEAR = 2014;
int LED[LEDMAX] = {
  42, 41, 40, 39, 38, 18, 19, 20, 21, 22, 23, 24, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3
};

void setup() {
  // Start a serial connection to console
  Serial.begin(9600);
  
  // Start a wire connection to the RTC
  Wire.begin();
  
  // Set the time
  // Only run this once to initially set the clock
  // SetRTC();
  
  // Set all LEDs as output
  for(int i = 0; i < LEDMAX; i++) {
    pinMode(LED[i], OUTPUT);
  }
  
  // Set power switch as input
  pinMode(SWITCH, INPUT);
  
  // Clear the LEDs
  LedClear();
}

void loop() {
  // See if the power switch is in the 'on' or 'off' position
  if(digitalRead(SWITCH) == HIGH) {
    POWER = true;
  } else {
    POWER = false;
  }
  
  // Determine action based on power switch
  if(POWER) {
    // Retrieve the time
    GetTime();
    
    // Set the LEDs
    LedSetNoon(TIME_HOURS);
  } else {
    // Clear the LEDs
    LedClear();
  }
  
  // Delay the loop for a bit
  delay(250);
}

void LedSetNoon(int _NOON) {
  // Account for zero-index... Grr, I'm better than this crap
  _NOON = _NOON - 1;
  
  // If noon is a negative, convert to a positive based on 24-hour clock
  if(_NOON < 0) {
    _NOON = 23;
  }
  
  // Define some variables and temporary arrays
  int LedSlice[LEDMAX], LedRemain[LEDMAX], LedFinal[LEDMAX];
  int LedSliceSize, LedRemainSize;
  int LedFinalIncrement = 0;
  int a = 0, b = 0;
  
  // Cut out the hours that are less than _NOON into a temporary array, leave the remainders in another
  for(int x = 0; x < LEDMAX; x++) {
    if(x < _NOON) {
      LedSlice[a] = LED[x];
      a++;
    } else {
      LedRemain[b] = LED[x];
      b++;
    }
  }
  
  LedSliceSize = a;
  LedRemainSize = b;
  
  // Merge the remaining hours into the final temporary array
  for(int y = 0; y < LedRemainSize; y++, LedFinalIncrement++) {
    LedFinal[LedFinalIncrement] = LedRemain[y];
  }
  
  // Merge the sliced hours into the final temporary array
  for(int z = 0; z < LedSliceSize; z++, LedFinalIncrement++) {
    LedFinal[LedFinalIncrement] = LedSlice[z];
  }
  
  // Clear the lights in preparation for new data
  LedClear();
  
  // If the hours are within the first or last indices of the array, light 'em up (noon is 0 index)
  for(int i = 0; i < LEDMAX; i++) {
    if(i <= 5 || i >= 19) {
      digitalWrite(LedFinal[i], HIGH);
    }
  }
}

void LedClear() {
  // Clear all the LEDs
  for(int i = 0; i < LEDMAX; i++) {
    digitalWrite(LED[i], LOW);
  }
}

void SetRTC() {
  // Set the time and then the date
  SetTime();
  SetDate();
}

void SetDate() {
  // Begin the transmission
  Wire.beginTransmission(104);
  
  // Start at the 3rd position on the register (DAY)
  Wire.write(3);
  
  // Send the data
  Wire.write(DecimalToBytecode(TIME_DAY));
  Wire.write(DecimalToBytecode(TIME_DATE));
  Wire.write(DecimalToBytecode(TIME_MONTH));
  Wire.write(DecimalToBytecode(TIME_YEAR));
  
  // End the tranmission
  Wire.endTransmission();
}

void SetTime() {
  // Begin the transmission
  Wire.beginTransmission(104);
  
  // Start at the beginning of the register
  Wire.write(0);

  // Send the data
  Wire.write(DecimalToBytecode(TIME_SECONDS));
  Wire.write(DecimalToBytecode(TIME_MINUTES));
  Wire.write(DecimalToBytecode(TIME_HOURS));

  // End the tranmission
  Wire.endTransmission();
}

void GetDate() {
  // Begin the transmission
  Wire.beginTransmission(104);
  
  // Start at the 3rd position on the register (DAY)
  Wire.write(3);
  
  // End the transmission
  Wire.endTransmission();
  
  // Get 5 bytes (DAY, DATE, MONTH, YEAR, CONTROL)
  Wire.requestFrom(104, 4);
  
  // Set the variables
  TIME_DAY   = BytecodeToDecimal(Wire.read());
  TIME_DATE  = BytecodeToDecimal(Wire.read());
  TIME_MONTH = BytecodeToDecimal(Wire.read());
  TIME_YEAR  = BytecodeToDecimal(Wire.read());
}

void GetTime() {
  // Begin the transmission
  Wire.beginTransmission(104);
  
  // Start at the beginning of the register
  Wire.write(0);
  
  // End the transmisson
  Wire.endTransmission();
  
  // Get 3 bytes (SECONDS, MINUTES, HOURS)
  Wire.requestFrom(104, 3);
  
  // Set the variables
  TIME_SECONDS = BytecodeToDecimal(Wire.read() & 0x7f);
  TIME_MINUTES = BytecodeToDecimal(Wire.read());
  TIME_HOURS   = BytecodeToDecimal(Wire.read() & 0x3f);
}

byte DecimalToBytecode(byte val) {
  // Convert decimal to bytecode
  return ((val / 10 * 16) + (val % 10));
}

byte BytecodeToDecimal(byte val) {
  // Convert bytecode to decimal
  return ((val / 16 * 10) + (val % 16));
}

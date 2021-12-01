#include <Arduino.h>
#include <EEPROM.h>

uint16_t i = 0;
uint16_t eeprom_address = 0;
int16_t sensor_value = 0;
int16_t sensor_value_old = 0;
int16_t sensor_value_diff = 0;
int16_t sensor_value_diff_old = 0;
int8_t sensor_value_diff_diff = 0;
bool takeoff = false;
const int reed_switch = 2;

void setup() {
  pinMode(reed_switch, INPUT);

  if (digitalRead(reed_switch)/*the switch is pulled up*/) {
    while (1)/*Dump it all*/
    {
      Serial.begin(9600);
      for(eeprom_address = 0; eeprom_address < EEPROM.length(); eeprom_address++) {
        Serial.print(EEPROM.read(eeprom_address), DEC);
        Serial.print(",");
      }
      delay(15000);
    }
  }

  // TIMER 1 for interrupt frequency 10 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 10 Hz increments
  OCR1A = 24999; // = 16000000 / (64 * 10) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 64 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
}

void loop() {
  //none
}


ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0; // reset counter
  
  
  // get height
  sensor_value = {
    // Sensor read here
    0
  };

  // takeoff detection
  if ((sensor_value > sensor_value_old + 2) && takeoff == 0) {
    // send takeoff signal
    takeoff = 1;
  }
  else{
    // calculate rocket velocity
    sensor_value_diff = sensor_value - sensor_value_old;
    
    // calculate rocket acceletion
    sensor_value_diff_diff = sensor_value_diff - sensor_value_diff_old;
    
    // store acceletion in EEPROM
    // we could figure out the displacement by deriving the acceleration
    EEPROM.update(eeprom_address, sensor_value_diff_diff);
    
    // shift the 'C's
    sensor_value_old = sensor_value;
    sensor_value_diff_old = sensor_value_diff;

    eeprom_address++;
    i++;

    // break out of the loop if we have reached the end of the EEPROM
    if (i >= EEPROM.length()) {
      // turn off timer 1
      TCCR1B = 0;
      // turn off timer 1 interrupt
      TIMSK1 = 0;


      // optionally print the EEPROM content (for debugging)

        // for(eeprom_address = 0; eeprom_address < EEPROM.length(); eeprom_address++) {
        //   Serial.print(EEPROM.read(eeprom_address), DEC);
        //   Serial.print(" ");
        // }

    }
  }
}

class Controller {
  public:
    Controller() {}

    void init(byte data, byte cmd, byte clk, byte ss, byte led) {
      // Data pin with internal pull-up
      pinMode(data, INPUT_PULLUP);
      //digitalWrite(data, HIGH);
      this->data = data;

      // Command pin
      pinMode(cmd, OUTPUT);
      this->cmd = cmd;

      // Clock pin with high idle
      pinMode(clk, OUTPUT);
      digitalWrite(clk, HIGH);
      this->clk = clk;

      // Chip select pins with high idle
      pinMode(ss, OUTPUT);
      digitalWrite(ss, HIGH);
      this->ss = ss;
      
      // LED indicators
      pinMode(led, OUTPUT);
      this->led = led;
    }
    
    void poll() {
      delay(1);
      
      // Bring SS low to start communication
      digitalWrite(ss, LOW);
      exchange(0x01); // START
      exchange(0x42); // BEGIN READ
      exchange(0xFF); // WAIT BEGIN READ
      byte buttonData1 = ~exchange(0xFF); // RESPONSE BYTE 1
      byte buttonData2 = ~exchange(0xFF); // RESPONSE BYTE 1
      // Bring SS high to end communication
      digitalWrite(ss, HIGH);

      // int mask of button data
      int buttonData = (buttonData2 << 8) | buttonData1;

      // Light the LED if a button is being pressed
      digitalWrite(led, buttonData ? HIGH : LOW);

      // Set the joystick buttons
      for (int i = 0; i < 16; i++)
        Joystick.button(i + 1, buttonData & (1 << i));
    }

  private:
    byte clockDelay = 50;

    byte exchange(byte dataOut) {
      byte temp;
      byte dataIn = 0;
      
      // Loop over the data least significant bit first
      for (int i = 0; i < 8; i++) {
        // Either send a high or a low for the corresponding data bit
        if (dataOut & (1 << i))
          digitalWrite(cmd, HIGH);
        else
          digitalWrite(cmd, LOW);
        // Data is transfered on the falling edge of the clock
        digitalWrite(clk, LOW);
        
        delayMicroseconds(clockDelay);

        // Read the data line to get a response data bit
        temp = digitalRead(data);
        if (temp)
          dataIn |= 1 << (7 - i);
        // Data is read on the rising edge of the clock
        digitalWrite(clk, HIGH);

        delayMicroseconds(clockDelay);
      }
      return dataIn;
    }

    byte data;
    byte cmd;
    byte clk;
    byte ss;
    byte led;
};

#define DATA 15
#define CMD 14
#define CLK 13
#define CS 12
#define LED 11

Controller ctrl;

void setup() {
  ctrl.init(DATA, CMD, CLK, CS, LED);
}

void loop()
{    
  ctrl.poll();
}


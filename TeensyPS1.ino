class TeensyPS1 {
  public:
    TeensyPS1() {}

    void init(byte data, byte cmd, byte clk, byte cs[], byte led[], byte numControllers) {
      // Data pin with internal pull-up
      pinMode(data, INPUT_PULLUP);
      this->data = data;

      // Command pin
      pinMode(cmd, OUTPUT);
      this->cmd = cmd;

      // Clock pin with high idle
      pinMode(clk, OUTPUT);
      digitalWrite(clk, HIGH);
      this->clk = clk;

      this->numControllers = numControllers;
      for (int i = 0; i < numControllers; i++) {
        // Chip select pins with high idle
        pinMode(cs[i], OUTPUT);
        digitalWrite(cs[i], HIGH);
        this->cs[i] = cs[i];

        // LED Indicators
        pinMode(led[i], OUTPUT);
        this->led[i] = led[i];
      }
    }
    
    void poll() {
      delay(16);

      int buttons = 0;
      byte buttonData1 = 0;
      byte buttonData2 = 0;

      for (int i = 0; i < numControllers; i++) {
        // Bring CS low to start communication
        digitalWrite(cs[i], LOW);
        exchange(0x01); // START
        exchange(0x42); // BEGIN READ
        exchange(0xFF); // WAIT BEGIN READ
        buttonData1 = ~exchange(0xFF); // RESPONSE BYTE 1
        buttonData2 = ~exchange(0xFF); // RESPONSE BYTE 2
        // Bring SS high to end communication
        digitalWrite(cs[i], HIGH);

        // 16b bitfield of all button states
        buttons = (buttonData2 << 8) | buttonData1;

        // Light the corresponding LED if any button is pressed
        digitalWrite(led[i], (bool)buttons);

        for (int j = 0; j < 16; j++) {
          Joystick.button((i * 16) + j + 1, (bool)(buttons & (1 << j)));
        }
      }
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
    byte cs[2];
    byte led[2];
    byte numControllers;
};

#define DATA 13
#define CMD 14
#define CLK 15
#define CS1 12
#define LED1 11
#define CS2 10
#define LED2 9

TeensyPS1 joystick;

void setup() {
  byte cs[] = {CS1, CS2};
  byte led[] = {LED1, LED2};
  joystick.init(DATA, CMD, CLK, cs, led , 2);
}

void loop() {    
  joystick.poll();
}

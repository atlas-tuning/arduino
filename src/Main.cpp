#include <Arduino.h>
#include <esp32_can.h>

uint firmware_version = 0x01;

#define SYS_LED 2
#define RX_LED GPIO_NUM_26
#define TX_LED GPIO_NUM_27

bool rx_lit = false;
bool tx_lit = false;

void setup() {
  Serial.setTxBufferSize(1024);
  Serial.begin(460800);

  CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5);

  pinMode(SYS_LED, OUTPUT);
  pinMode(RX_LED, OUTPUT);
  pinMode(TX_LED, OUTPUT);

  digitalWrite(SYS_LED, HIGH);
  delay(50);
  digitalWrite(SYS_LED, LOW);
}

uint readSerial32() {
  uint i_0 = (uint) (Serial.read() & 0xFF);
  uint i_1 = (uint) (Serial.read() & 0xFF);
  uint i_2 = (uint) (Serial.read() & 0xFF);
  uint i_3 = (uint) (Serial.read() & 0xFF);

  uint i = i_3;
  i += i_2 << 8;
  i += i_1 << 16;
  i += i_0 << 24;

  return i;
}

ushort readSerial16() {
  uint i_0 = (uint) (Serial.read() & 0xFF);
  uint i_1 = (uint) (Serial.read() & 0xFF);

  ushort i = i_1;
  i += i_0 << 8;
  
  return i;
}

void handle0x1_Ping() {
  Serial.write(Serial.read());
}

void handle0x2_Version() {
  Serial.write(0x2);

  Serial.write((firmware_version >> 24) & 0xFF);
  Serial.write((firmware_version >> 16) & 0xFF);
  Serial.write((firmware_version >> 8) & 0xFF);
  Serial.write(firmware_version & 0xFF);
}

void handle0x10_Open() {
  uint baud = readSerial32();
  int listenOnly = Serial.read() & 0xFF;
  int filters = Serial.read() & 0xFF;
  
  CAN0.enable();
  CAN0.begin(baud, 255);

  for (int filter = 0; filter < filters; filter++) {
    uint id = readSerial32();
    uint mask = readSerial32();

    bool extended;
    if (id >= 0x7FF) extended = true;
    else extended = false;

    if (CAN0._setFilter(id, mask, extended) == -1) {
      CAN0.disable();
      Serial.write(0xFE);
      return;
    }
  }

  Serial.write(0x10);
}


void handle0x1F_Close() {
  CAN0.disable();
  Serial.write(0x1F);
}


void handle0xD0_Tx() {
  CAN_FRAME frame;

  frame.id = readSerial32();
  frame.rtr = 0;
  if (frame.id >= 0x7FF) frame.extended = true;
  else frame.extended = false;

  frame.length = 8;

  twai_message_t __TX_frame;

  __TX_frame.identifier = frame.id;
  __TX_frame.data_length_code = frame.length;
  __TX_frame.rtr = frame.rtr;
  __TX_frame.extd = frame.extended;
  
  for (int i = 0; i < 8; i++) __TX_frame.data[i] = Serial.read();

  switch (twai_transmit(&__TX_frame, pdMS_TO_TICKS(4)))
  {
    case ESP_OK:
        digitalWrite(TX_LED, HIGH); 
        delay(50);
        digitalWrite(TX_LED, LOW);
        break;
    case ESP_ERR_TIMEOUT:
        break;
    case ESP_ERR_INVALID_ARG:
    case ESP_FAIL:
    case ESP_ERR_INVALID_STATE:
    case ESP_ERR_NOT_SUPPORTED:
        break;
  }
    
}

void readSerialCommand() {
  uint command = Serial.read() & 0xFF;

  switch (command) {
    case 0x1:
      handle0x1_Ping();
      break;
    case 0x2:
      handle0x2_Version();
      break;
    case 0x10:
      handle0x10_Open();
      break;
    case 0x1F:
      handle0x1F_Close();
      break;
    case 0xD0:
      handle0xD0_Tx();
      break;
    default: 
      Serial.write(0xFF);
      Serial.write(command);
      break;
  }

  Serial.flush();
}

void readCANFrame(CAN_FRAME &frame) {
  byte packet[17];
  packet[0] = 0xD1;

  uint32_t timestamp = frame.timestamp;
  packet[1] = timestamp >> 24 & 0xFF;
  packet[2] = timestamp >> 16 & 0xFF;
  packet[3] = timestamp >> 8 & 0xFF;
  packet[4] = timestamp & 0xFF;

  uint32_t id = frame.id;
  packet[5] = id >> 24 & 0xFF;
  packet[6] = id >> 16 & 0xFF;
  packet[7] = id >> 8 & 0xFF;
  packet[8] = id & 0xFF;

  uint length = frame.length;
  int i = 0;
  for (; i < length && i < 8; i ++) {
    packet[9 + i] = frame.data.byte[i];
  }
  for (; i < 8; i ++) {
    packet[9 + i] = 0;
  }

  if ((frame.data.byte[0] & 0xF0) == 0x10) {
    // Send flow control frame
    twai_message_t __TX_frame;

    __TX_frame.identifier = id - 0x8;
    __TX_frame.data_length_code = 8;
    __TX_frame.rtr = 0;
    __TX_frame.extd = frame.extended;
    __TX_frame.data[0] = 0x30;
    __TX_frame.data[1] = 0x0;
    __TX_frame.data[2] = 0x0;

    twai_transmit(&__TX_frame, pdMS_TO_TICKS(4));
  } else if ((packet[9] & 0xF0) == 0x30) {
    return;
  }

  portDISABLE_INTERRUPTS();
  portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
  taskENTER_CRITICAL(&myMutex);
  Serial.write(packet, 17);
  Serial.flush(true);
  taskEXIT_CRITICAL(&myMutex);
  portENABLE_INTERRUPTS();

  digitalWrite(RX_LED, HIGH); 
  delay(50);
  digitalWrite(RX_LED, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    readSerialCommand();
  }

  CAN_FRAME frame;
  if (CAN0.rx_avail() && CAN0.read(frame)) {
    readCANFrame(frame);
  }
}
#include <Arduino.h>
#include <esp32_can.h>

#include <vector>
#include <map>

uint firmware_version = 0x01;

#define SYS_LED 2 // Blue (top)
#define RX_LED GPIO_NUM_26 // Green
#define TX_LED GPIO_NUM_27 // Blue

typedef unsigned char ubyte;



struct ISOTPFrame {
  mutable uint id;

  mutable uint index;
  mutable int next_fc_index;

  mutable int64_t timestamp;
  mutable int64_t st_min;
  mutable int64_t next_frame_time;

  mutable ushort available;
  mutable ushort position;

  char* data;
  mutable ushort length;
};

std::vector<ISOTPFrame*> tx_buffer;
std::map<uint, ISOTPFrame*> rx_buffer;

bool rx_lit = false;
bool tx_lit = false;

bool can_installed = false;

void setup() {
  pinMode(SYS_LED, OUTPUT);

  digitalWrite(SYS_LED, HIGH);
  delay(50);
  digitalWrite(SYS_LED, LOW);

  pinMode(RX_LED, OUTPUT);
  pinMode(TX_LED, OUTPUT);

  digitalWrite(RX_LED, LOW);
  digitalWrite(TX_LED, LOW);

  Serial.setRxBufferSize(1024);
  Serial.setTxBufferSize(1024);
  Serial.begin(921600);

  CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5);
}

uint readSerial32() {
  while (Serial.available() < 4) {
  }

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
  while (Serial.available() < 2) {
  }

  uint i_0 = (uint) (Serial.read() & 0xFF);
  uint i_1 = (uint) (Serial.read() & 0xFF);

  ushort i = i_1;
  i += i_0 << 8;
  
  return i;
}

void handle0x1_Ping() {
  while (Serial.available() < 1) {
  }
  
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
  
  while (Serial.available() < 2) {
  }
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
  uint id = readSerial32();
  ushort length = readSerial16();

  char* buffer = (char*) malloc(sizeof(char) * length);

  int read;
  int ready = min((int)length, Serial.available());
  if (ready > 0) {
    read = Serial.readBytes(buffer, ready);
    if (read < 0) {
      read = 0;
    }
  } else {
    read = 0;
  }

  ISOTPFrame* frame = new ISOTPFrame { id, 0, 0, esp_timer_get_time(), 0, 0, (ushort)read, 0, buffer, length };
  tx_buffer.push_back(frame);
}

void readSerialCommand() {
  for(auto & tx_frame : tx_buffer) {
    int remaining = tx_frame->length - tx_frame->available;
    if (remaining <= 0) {
      continue;
    }

    int available = Serial.available();
    int ready = min(remaining, available);
    int read = 0;
    while (read < ready) {
      int b = Serial.read();
      if (b < 0) {
        break;
      }

      tx_frame->data[tx_frame->available + read] = b;
      read++;
    }

    tx_frame->available += (ushort)read;

    return;
  }

  uint command = Serial.read() & 0xFF;
  if (command < 0) {
    return;
  }

  switch (command) {
    case 0x1:
      handle0x1_Ping();
      break;
    case 0x2:
      handle0x2_Version();
      break;
    case 0x3: // End of text
      return;
    case 0x10:
      handle0x10_Open();
      break;
    case 0x1F:
      handle0x1F_Close();
      break;
    case 0xD0:
      handle0xD0_Tx();
      return;
    default: 
      Serial.write(0xFF);
      Serial.write(command);
      break;
  }

  Serial.flush(true);
}

void handleRxFrame(CAN_FRAME &frame) {
  if ((frame.data.byte[0] & 0xF0) == 0x30) {
    // Handle flow control frame
    if (tx_buffer.size() == 0) {
      return;
    }

    ISOTPFrame* tx_frame = tx_buffer.front();
    if (tx_frame->id != frame.id - 8) {
      return;
    }

    char fc_flag = frame.data.byte[0] & 0x0F;

    ubyte bs = frame.data.byte[1] & 0xFF;
    uint st_min = frame.data.byte[2] & 0xFF;

    if (st_min == 0) {
      // Do nothing
    } else if (st_min > 0 && st_min < 0xF1) {
      st_min *= 1000;
    } else {
      st_min = (st_min - 0xF0) * 100;
    }
    tx_frame->st_min = st_min;

    uint64_t next_frame_time;
    switch (fc_flag) {
      case 0x0:
        next_frame_time = 0;
        break;
      case 0x1:
        next_frame_time = esp_timer_get_time() + st_min;
        break;
      case 0x2:
        tx_buffer.erase(tx_buffer.begin());
        free(tx_frame->data);
        free(tx_frame);
        return;
    }

    tx_frame->next_frame_time = next_frame_time;

    if (bs > 0) {
      tx_frame->next_fc_index = tx_frame->index + bs;
    } else {
      tx_frame->next_fc_index = -1;
    }

    return;
  } else if ((frame.data.byte[0] & 0xF0) == 0x10) { // First frame
    // Send flow control frame
    twai_message_t __TX_frame;

    __TX_frame.identifier = frame.id - 0x8;
    __TX_frame.data_length_code = 8;
    __TX_frame.rtr = 0;
    __TX_frame.extd = frame.extended;
    __TX_frame.data[0] = 0x30;
    __TX_frame.data[1] = 0x0;
    __TX_frame.data[2] = 0x0;

    twai_transmit(&__TX_frame, pdMS_TO_TICKS(1000));

    // Handle first frame
    if (rx_buffer.count(frame.id)) {
      ISOTPFrame* rx_frame = rx_buffer[frame.id];
      rx_buffer.erase(rx_frame->id);
      free(rx_frame->data);
      free(rx_frame);
    }

    ushort length = ((frame.data.byte[0] & 0x0F) << 8) | frame.data.byte[1];
    char* buffer = (char*) malloc(sizeof(char) * length);

    uint i = 0;
    for (; i < frame.length - 2; i++) {
      buffer[i] = frame.data.byte[i + 2];
    }

    ISOTPFrame* rx_frame = new ISOTPFrame { frame.id, 1, 0, esp_timer_get_time(), 0, 0, (ushort) (i), (ushort) (i), buffer, length };
    rx_buffer[frame.id] = rx_frame;
    return;
  }
  
  uint id = frame.id;
  char* packet;
  ushort length;

  if ((frame.data.byte[0] & 0xF0) == 0x00) { // Single frame
    length = frame.data.byte[0] & 0x0F;
    packet = new char[1 + 4 + 2 + length];
    for (int i = 0; i < length && i < frame.length - 1; i ++) {
      packet[i + 7] = frame.data.byte[i + 1];
    }
  } else if ((frame.data.byte[0] & 0xF0) == 0x20) { // Consecutive frame
    // Handle first frame
    if (rx_buffer.count(frame.id) == 0) {
      return;
    }

    ISOTPFrame* rx_frame = rx_buffer[frame.id];

    ubyte index = frame.data.byte[0] & 0x0F;
    if (rx_frame->index != index) {
      rx_buffer.erase(rx_frame->id);
      free(rx_frame->data);
      free(rx_frame);
      return;
    }

    rx_frame->index += 1;
    if (rx_frame->index > 0x0F) {
      rx_frame->index = 0;
    }

    ushort remaining = rx_frame->length - rx_frame->position;
    uint i = 0;
    for (; i < frame.length - 1 && i < remaining; i ++){
      rx_frame->data[rx_frame->position + i] = frame.data.byte[i + 1];
    }

    rx_frame->position += i;
    rx_frame->available += i;

    if (rx_frame->position < rx_frame->length) {
      rx_buffer[frame.id] = rx_frame;
      return;
    }

    length = rx_frame->length;
    packet = new char[1 + 4 + 2 + length];
    for (int i = 0; i < length; i++) {
      packet[7 + i] = rx_frame->data[i];
    }

    rx_buffer.erase(rx_frame->id);
    free(rx_frame->data);
    free(rx_frame);
  }

  
  packet[0] = 0xD1;
  packet[1] = (id >> 24) & 0xFF;
  packet[2] = (id >> 16) & 0xFF;
  packet[3] = (id >> 8) & 0xFF;
  packet[4] = id & 0xFF;
  packet[5] = (length >> 8) & 0xFF;
  packet[6] = length & 0xFF;

  Serial.write(packet, 1 + 4 + 2 + length);
  Serial.flush(true);

  free(packet);
}

bool handleTxBuffer() {
  if (tx_buffer.size() == 0) {
    return false;
  }

  ISOTPFrame* frame = tx_buffer.front();
  if (frame->index != 0 && frame->next_fc_index >= 0 && frame->index >= frame->next_fc_index) {
    return false;
  } else if (frame->available == 0) {
    return false;
  }

  if (frame->next_frame_time > 0) {
    uint64_t now = esp_timer_get_time();
    if (now < frame->next_frame_time) {
      return false;
    }
  }

  uint can_length = 8;
  bool first = frame->position == 0;
  int remaining = frame->length - frame->position;

  twai_message_t __TX_frame;
  __TX_frame.identifier = frame->id;
  __TX_frame.data_length_code = can_length;
  __TX_frame.rtr = 0;

  bool extended;
  if (frame->id >= 0x7FF) extended = true;
  else extended = false;
  __TX_frame.extd = extended;

  uint can_position = 0;
  if (!first) {
    // Consecutive frame
    __TX_frame.data[0] = 0x20 | (frame->index & 0x0F);
    can_position = 1;
  } else if (remaining <= 7) {
    // Single frame
    __TX_frame.data[0] = 0x00 | (remaining & 0x07);
    can_position = 1;
  } else {
    // First frame
    __TX_frame.data[0] = 0x10 | ((frame->length >> 8) & 0x0F);
    __TX_frame.data[1] = frame->length & 0xFF;
    can_position = 2;
  }

  int window = min((int) (can_length - can_position), frame->length - frame->position);

  if (frame->available < frame->length && frame->available < frame->position + window) {
    // Cannot fill the buffer yet
    return false;
  }

  for (uint frame_position = 0; frame_position < window; frame_position ++) {
    __TX_frame.data[can_position + frame_position] = frame->data[frame->position + frame_position];
  }

  esp_err_t res = twai_transmit(&__TX_frame, pdMS_TO_TICKS(1000));
  switch (res)
  {
    case ESP_ERR_TIMEOUT:
    case ESP_FAIL:
      return false;
    case ESP_OK:
      break;
    default:
      Serial.write(0xFE);
      Serial.flush(true);
      break;
  }

  frame->position += window;

  if (frame->position >= frame->length) {
    tx_buffer.erase(tx_buffer.begin());
    free(frame->data);
    free(frame);
    return true;
  }

  frame->index ++;

  if (frame->st_min > 0) {
    frame->next_frame_time = esp_timer_get_time() + frame->st_min;
  } else {
    frame->next_frame_time = 0;
  }

  return true;
}

void loop() {
  digitalWrite(SYS_LED, HIGH);

  if (Serial.available() > 0) {
    readSerialCommand();
  }

  CAN_FRAME frame;
  if (CAN0.rx_avail() && CAN0.read(frame)) {
    handleRxFrame(frame);
  }

  
  digitalWrite(RX_LED, rx_buffer.size() > 0);
  digitalWrite(TX_LED, tx_buffer.size() > 0);

  handleTxBuffer();

  digitalWrite(SYS_LED, LOW);
}
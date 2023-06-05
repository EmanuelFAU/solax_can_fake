#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>
// CAN0
#define SPI_CS_CAN0 14
#define CAN0_INT 32

// CAN1
#define SPI_CS_CAN1 21
#define CAN1_INT 27

long unsigned int rxId;
long unsigned int rxId_CAN1;
unsigned char len = 0;
unsigned char len_CAN1 = 0;
unsigned char rxBuf[8];
unsigned char rxBuf_CAN1[8];
MCP_CAN CAN0(SPI_CS_CAN0); // Set CS to pin 10 (nodemcu = 5, nano = 10)
MCP_CAN CAN1(SPI_CS_CAN1); // Set CS to pin 10 (nodemcu = 5, nano = 10)

uint8_t msgStoreBuf[10][16];
unsigned long msgStoreId[10];
uint8_t msgStoreLen[10];
uint8_t numMsgs = 0;

void print_CAN_frame(unsigned long id, unsigned char len, unsigned char *buf);

void read_and_store_CAN_msg(MCP_CAN *can, uint8_t int_pin);
void send_CAN_frames(MCP_CAN *can, char name);

void setup()
{
  Serial.begin(115200);
  delay(2000);

  // CAN 0
  // if (CAN0.begin(MCP_EXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) // seems to be a bug in the silicon
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 CAN 0 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515 CAN 0...");
  CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.
  // CAN0.setMode(MCP_LISTENONLY); // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

  // CAN 1
  if (CAN1.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 CAN 1 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515 CAN 1...");
  CAN1.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.
  // CAN1.setMode(MCP_LISTENONLY); // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN1_INT, INPUT); // Configuring pin for /INT input

  Serial.println("Solax Pylontech bridge started");

  delay(1000);

  // byte data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  // send_CAN0_frame(0x100, 8, data);
}

void loop()
{
  read_and_store_CAN_msg(&CAN0, CAN0_INT);
  send_CAN_frames(&CAN1, 'B');

  read_and_store_CAN_msg(&CAN1, CAN1_INT);
  send_CAN_frames(&CAN0, 'A');
}

void send_CAN_frame(MCP_CAN *can, unsigned long id, unsigned char len, unsigned char *buf)
{
  byte sendStatus = can->sendMsgBuf(id, 1, len, buf);

  if (sendStatus != CAN_OK)
  {
    Serial.println("Error Sending Message...");
  }
}

void send_CAN_frames(MCP_CAN *can, char name)
{
  if (numMsgs > 0)
  {
    for (uint8_t i = 0; i < numMsgs; i++)
    {
      Serial.print(name);
      Serial.print(": ");
      Serial.print(msgStoreId[i], HEX);
      Serial.print(" ");
      for (uint8_t j = 0; j < msgStoreLen[i]; j++)
      {
        Serial.print(msgStoreBuf[i][j], HEX);
        Serial.print(" ");
      }
      Serial.println();

      if (msgStoreId[i] == 0x80001877)
      {
        Serial.print("0x1877 ");
        if (msgStoreBuf[i][4] == 0x01)
        {
          Serial.print("replaced 0x01 to ");
        }
        msgStoreBuf[i][4] = 0x54;
        Serial.print(msgStoreBuf[i][4], HEX);
        Serial.println();
      }
      send_CAN_frame(can, msgStoreId[i], msgStoreLen[i], msgStoreBuf[i]);
      delayMicroseconds(200);
    }
    numMsgs = 0;
  }
}

void read_and_store_CAN_msg(MCP_CAN *can, uint8_t int_pin)
{
  int continueReading = 1;
  long startedReading = micros();
  byte dataAvailabe = 0;
  while (continueReading == 1)
  {
    if (!digitalRead(int_pin)) // If CAN0_INT pin is low, read receive buffer
    {
      dataAvailabe = 1;
      startedReading = micros();
      can->readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
      msgStoreId[numMsgs] = rxId;
      msgStoreLen[numMsgs] = len;

      if (len > 8)
      {
        Serial.println("ERROR: len > 8");
      }

      for (int i = 0; i < len; i++)
      {
        msgStoreBuf[numMsgs][i] = rxBuf[i];
      }
      numMsgs++;
      if (numMsgs > 9)
      {
        Serial.println("ERROR: numMsgs > 9");
        numMsgs = 0;
      }
    }
    else if (dataAvailabe == 1)
    {
      if (micros() - startedReading > 1000)
      { // if no new message for 1ms, stop reading
        continueReading = 0;
        dataAvailabe = 0;
      }
    }
    else
    {
      continueReading = 0;
    }
  }
}

void print_CAN_frame(unsigned long id, unsigned char len, unsigned char *buf)
{
  Serial.print("ID: ");
  Serial.print(id, HEX);
  Serial.print("  Data: ");
  for (int i = 0; i < len; i++) // print the data
  {
    Serial.print(buf[i], HEX);
    Serial.print("\t");
  }
  Serial.println();
}

// CAN readout of VW type 5 modules
//

#include <FlexCAN_T4.h>
#include <SPI.h>

unsigned char len = 0;
char msgString[128];                        // Array to store serial string
unsigned long looptime = 0;


FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;  // Can1 port 

int controlid = 0x0BA;
int moduleidstart = 0x1C0;


uint16_t voltage[30][14];
int modulespresent = 5;
int sent = 0;

int debug = 0;
CAN_message_t msg;

void setup()
{
  Serial.begin(115200);

  Can1.begin();
  Can1.setBaudRate(500000);


  Serial.println("Time Stamp,ID,Extended,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8");
}

void loop()
{
  Can1.events();

  if ( Can1.read(msg) )                      //  read receive buffer
  {
    candecode();
  }
  if (Serial.available())
  {
    menu();
  }

  if (millis() > looptime + 500)
  {
    looptime = millis();
    for (int y = 0; y < modulespresent; y++)
    {
      Serial.println();
      Serial.print("Module ");
      Serial.print(y+1);
      Serial.print(" Voltages : ");
      for (int i = 1; i < 13; i++)
      {
        Serial.print(voltage[y][i]);
        Serial.print("mV ");
      }
    }

    Serial.println();
    sendcommand();
  }

}

void menu()
{
  byte incomingByte = Serial.read(); // read the incoming byte:

  switch (incomingByte)
  {
    case 's': //
      sendcommand();
      break;
  }
}

void sendcommand()
{
  msg.id =controlid;
  msg.len = 8;
  msg.flags.extended = 0;
  msg.buf[0] = 0x00;
  msg.buf[1] = 0x00;
  msg.buf[2] = 0x00;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = 0x00;
  msg.buf[7] = 0x00;
  Can1.write(msg);
  delay(50);

  msg.id =controlid;
  msg.len = 8;
  msg.flags.extended = 0;
  msg.buf[0] = 0x45;
  msg.buf[1] = 0x01;
  msg.buf[2] = 0x28;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = 0x00;
  msg.buf[7] = 0x30;
  Can1.write(msg);
  sent = 1;
  Serial.println();
  Serial.print("Command Sent");
  Serial.print(" Present Modules: ");
  Serial.print(modulespresent);
  Serial.println();
  modulespresent = 0;
}

void candecode()
{
  
  if (sent == 1)
  {
    moduleidstart = msg.id;
    sent = 0;
  }
  if (msg.id < 1024)
  {
    Serial.print("   ");
    int ID = msg.id - moduleidstart;
    switch (ID)
    {
      case 0:
        voltage[modulespresent][1] = uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000;
        voltage[modulespresent][3] = uint16_t(msg.buf[5] << 4) + uint16_t(msg.buf[4] >> 4) + 1000;

        voltage[modulespresent][2] = msg.buf[3] + uint16_t((msg.buf[4] & 0x0F) << 8) + 1000;
        voltage[modulespresent][4] = msg.buf[6] + uint16_t((msg.buf[7] & 0x0F) << 8) + 1000;
        break;
      case 1:
        voltage[modulespresent][5] = uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000;
        voltage[modulespresent][7] = uint16_t(msg.buf[5] << 4) + uint16_t(msg.buf[4] >> 4) + 1000;

        voltage[modulespresent][6] = msg.buf[3] + uint16_t((msg.buf[4] & 0x0F) << 8) + 1000;
        voltage[modulespresent][8] = msg.buf[6] + uint16_t((msg.buf[7] & 0x0F) << 8) + 1000;
        break;

      case 2:
        voltage[modulespresent][9] = uint16_t(msg.buf[1] >> 4) + uint16_t(msg.buf[2] << 4) + 1000;
        voltage[modulespresent][11] = uint16_t(msg.buf[5] << 4) + uint16_t(msg.buf[4] >> 4) + 1000;

        voltage[modulespresent][10] = msg.buf[3] + uint16_t((msg.buf[4] & 0x0F) << 8) + 1000;
        voltage[modulespresent][12] = msg.buf[6] + uint16_t((msg.buf[7] & 0x0F) << 8) + 1000;
        modulespresent++;
        sent =1;
        break;
    }
  }
  if (debug == 1)                        // If CAN0_INT pin is low, read receive buffer
  {
    Serial.print(millis());
    if ((msg.id & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (msg.id & 0x1FFFFFFF), msg.len);
    else
      sprintf(msgString, ",0x%.3lX,false,%1d", msg.id, msg.len);

    Serial.print(msgString);

    if ((msg.id & 0x40000000) == 0x40000000) {  // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for (byte i = 0; i < len; i++) {
        sprintf(msgString, ", 0x%.2X", msg.buf[i]);
        Serial.print(msgString);
      }
    }

    Serial.println();
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

// Import necessary file
#include <Arduino.h>
#include <SoftwareSerial.h>

// Defining the modem timeout
#define TIMEOUT 3000
#define DEV_ID 1

// Defining the post function
void postSiabApi(int, float);
void sim900a(int, float);

// Defining the modem command function
void modem_command(String);

// Setting the serial pin
SoftwareSerial mySerial(8, 7);

void setup()
{
  // Setting the baud rate
  Serial.begin(115200);
  mySerial.begin(19200);

  // Post to SIAB API
  // postSiabApi(DEV_ID, 1.44);
  sim900a(DEV_ID, 2);
}

void loop()
{
  // Baca flow
  // Jika udah jam 7 am maka {
  // postSiabApi(DEV_ID, flowmu_disini)
  // }
}

void sim900a(int device_id, float flow)
{
  modem_command("AT");
  modem_command("AT+CSQ");
  modem_command("AT+CGATT?");
  modem_command("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  modem_command("AT+SAPBR=3,1,\"APN\",\"indosatgprs\"");
  modem_command("AT+SAPBR=3,1,\"USER\",\"indosat\"");
  modem_command("AT+SAPBR=3,1,\"PWD\",\"indosat\"");

  modem_command("AT+SAPBR=1,1");
  modem_command("AT+SAPBR=2,1");
  modem_command("AT+CMEE=2");
  modem_command("AT+HTTPTERM");
  modem_command("AT+HTTPINIT");
  modem_command("AT+HTTPPARA=\"CID\",1");
  modem_command("AT+HTTPPARA=\"URL\",\"http://api.siagaairbersih.com/v1/post_sensor_by_device?flow=" + String(flow) + "&device_id=" + String(DEV_ID) + "\"");
  modem_command("AT+HTTPACTION=1");
  modem_command("AT+HTTPREAD");
  modem_command("AT+HTTPTERM");
  modem_command("AT+SAPBR=0,1");
}

void modem_command(String command)
{
  mySerial.println(command);
  Serial.println(command);
  while (mySerial.available() == 0)
    ; // wait for first char

  unsigned long lastRead = millis(); // last time a char was available
  while (millis() - lastRead < TIMEOUT)
  {
    while (mySerial.available())
    {
      Serial.write(mySerial.read());
      lastRead = millis(); // update the lastRead timestamp
    }
  }
  // No need for extra line feed since most responses contain them anyways
}

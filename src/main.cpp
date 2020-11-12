// Import necessary file
#include <Arduino.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

// Defining the modem timeout
#define DEV_ID 1
#define PIN_TX 8
#define PIN_RX 7
#define TIMEOUT 3000

// Defining the post function
void sim900a(int, float);

// Defining the modem command function
void modem_command(String);

void getReadings();
void clearEEPROM();
void pulseCounter();

// Setting the serial pin
SoftwareSerial mySerial(PIN_TX, PIN_RX);

byte sensorInterrupt = 0;
byte Water_Flow_Sensor_Pin = 2; //pin sensor water flow digital 2
String Data_Sensor;
float calibrationFactor = 4.5;
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;
unsigned long oldTime = 0;
volatile byte pulseCount = 0;

void setup()
{
  // clearEEPROM();
  Serial.println(totalMilliLitres);

  // Setting the baud rate
  Serial.begin(115200);
  mySerial.begin(19200);
  pinMode(Water_Flow_Sensor_Pin, INPUT);
  digitalWrite(Water_Flow_Sensor_Pin, HIGH);

  // Sensor aliran air terhubung ke pin 2 yang menggunakan interupsi 0. Dikonfigurasi untuk memicu pada perubahan status JATUH (transisi dari status HIGH ke status LOW)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  // Inisialisasi nilai total Mililiter
  EEPROM.get(1, totalMilliLitres);
  Serial.println("Nilai EEPROM Awal:");
  Serial.print("Jumlah Air yang keluar: ");
  Serial.println(totalMilliLitres);
}

void loop()
{
  getReadings();
  if (totalMilliLitres > 2000)
  {
    sim900a(DEV_ID, totalMilliLitres);
    clearEEPROM();
  }
}

void getReadings()
{
  if ((millis() - oldTime) > 1000)
  {
    //Nonaktifkan interupsi saat menghitung laju aliran dan mengirimkan nilainya ke host
    detachInterrupt(sensorInterrupt);

    //Rumus untuk menghitung laju aliran
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;

    //Menambahkan mililiter yang diteruskan dalam detik ini ke total kumulatif
    totalMilliLitres += flowMilliLitres;

    //Menyimpan nilai pada EEPROM
    EEPROM.put(1, totalMilliLitres);

    //Reset penghitung pulsa sehingga dapat melakukan penambahan ulang
    pulseCount = 0;

    //Mengaktifkan interupsi lagi setelah kita selesai mengirim output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    Serial.println(totalMilliLitres);
  }
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
  modem_command("AT+HTTPPARA=\"URL\",\"http://api.siagaairbersih.com/v1/post_sensor_by_device?flow=" + String(totalMilliLitres) + "&device_id=" + String(DEV_ID) + "\"");
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
  {
    getReadings();
  };

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

void pulseCounter()
{
  pulseCount++;
}

void clearEEPROM()
{
  for (unsigned int i = 0; i < EEPROM.length(); i++)
  {
    if (EEPROM.read(i) != 0) //skip already "empty" addresses
    {
      EEPROM.write(i, 0); //write 0 to address i
    }
  }
  EEPROM.get(1, totalMilliLitres);
  Serial.println("EEPROM erased");
}

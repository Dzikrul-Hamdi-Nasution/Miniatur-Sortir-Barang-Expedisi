#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <LiquidCrystal_I2C.h> // Library modul I2C LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <EEPROM.h>
#include "HX711.h"
#define DOUT  A0
#define CLK  A1
HX711 scale(DOUT, CLK);
float calibration_factor = 273;
float GRAM;


#define RST_PIN         8
#define SS_PIN          9         // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
Servo sortir;
Servo dorong;

int address_saldo_1 = 0;
int address_saldo_2 = 1;
int address_saldo_3 = 2;

String data_kartu;
byte IR = 28;
byte motor = 34;
String read_rfid;

String ID_1 = "8ad8cdb6";
String ID_2 = "c3e18b1a";
String ID_3 = "4c28b49";

int batas, tujuan;
int saldo1 = 0, saldo2 = 0, saldo3 = 0;
char nilai_top_up;

String dataIn;
String dt[10];
int i, kunci;
boolean parsing = false;

float harga_palembang = 6000;
float harga_makassar = 7000;
float harga_jakarta = 8000;
float biaya_tambahan = 5000;

int harga_fix_jakarta;
int harga_fix_palembang;
int harga_fix_makassar;

void setup(void) {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  dorong.attach(32);
  sortir.attach(30);
  dorong.write(30);//satndby
  lcd.backlight();
  lcd.init();
  pinMode(IR, INPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  scale.set_scale();
  scale.tare();
}



int berat_lcd;
void loop(void) {


  //dorong.write(30);//satndby
  //dorong.write(110);//dorong

  cek_berat();
  berat_lcd = GRAM;
  if (berat_lcd < 1) {
    lcd.setCursor(8, 0);
    lcd.print("    ");
  }
  lcd.setCursor(0, 0);
  lcd.print("Berat : ");
  lcd.setCursor(8, 0);
  lcd.print(berat_lcd);
  lcd.setCursor(0, 1);
  lcd.print("Harga : ");
  //lcd.print(harga);
  lcd.setCursor(0, 2);
  lcd.print("Tujuan : ");

  cek_kartu();

  int dataterkirim = Serial.read();
  if (dataterkirim == '1') {
    lcd.setCursor(0, 3);
    lcd.print("Tempelkan kartu");
    kunci = 2;

  }

  if (dataterkirim == '0') {
    lcd.clear();
    batas = 0;
    dataIn = "";
    while (batas < 2)
    {
      lcd.setCursor(0, 0);
      lcd.print("Tempelkan Kartu ");
      lcd.setCursor(0, 1);
      lcd.print("Untuk Top UP Saldo");
      cek_kartu();
      lcd.setCursor(0, 2);
      lcd.print("ID : ");
      lcd.setCursor(5, 2);
      lcd.print(data_kartu);
      lcd.setCursor(0, 3);
      lcd.print("Jumlah : ");
      lcd.setCursor(9, 3);
      lcd.print(dt[0].toInt());

      if (Serial.available() > 0)
      {
        kunci = 1;
        char inChar = (char)Serial.read();
        dataIn += inChar;
        if (inChar == '\n') {
          parsing = true;
        }
      }
      if (parsing)
      {
        parsingData();
        parsing = false;
        dataIn = "";

      }
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Saldo Anda");

    if (read_rfid == ID_1) {
      saldo1 = saldo1 + (dt[0].toInt());
      lcd.setCursor(0, 1);
      lcd.print(saldo1);
    }
    if (read_rfid == ID_2) {
      saldo2 = saldo2 + (dt[0].toInt());
      lcd.setCursor(0, 1);
      lcd.print(saldo2);
    }
    if (read_rfid == ID_3) {
      saldo3 = saldo3 + (dt[0].toInt());
      lcd.setCursor(0, 1);
      lcd.print(saldo3);
    }
    kunci = 0;
    delay(3000);
    lcd.clear();

  }


}


void cek_berat() {
  scale.set_scale(calibration_factor);
  GRAM = scale.get_units(), 4;
  //Serial.println(GRAM);
  delay(10);
}


void parsingData()
{
  int j = 0;
  dt[j] = "";
  for (i = 1; i < dataIn.length(); i++)
  {
    if ((dataIn[i] == '#') || (dataIn[i] == ','))
    {
      j++;
      dt[j] = ""; //inisialisasi variabel array dt[j]
    }
    else
    {
      dt[j] = dt[j] + dataIn[i];
    }
  }
}


void dump_byte_array(byte *buffer, byte bufferSize) {
  read_rfid = "";
  for (byte i = 0; i < bufferSize; i++) {
    read_rfid = read_rfid + String(buffer[i], HEX);
  }
}


void cek_kartu() {

  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  //Serial.println(read_rfid);

  if (kunci == 1) {
    batas = 4;
  }
  if (kunci == 2) {
    int tolak_akses = 0;
    dorong.write(110);//dorong
    delay(1000);
    cek_warna();
    delay(500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sisa Saldo Anda");
    if (read_rfid == ID_1) {
      if (tujuan == 1) {
        saldo1 = saldo1 - harga_fix_palembang;
      }
      if (tujuan == 2) {
        saldo1 = saldo1 - harga_fix_jakarta ;
      }
      if (tujuan == 3) {
        saldo1 = saldo1 - harga_fix_makassar;
      }
      if (saldo1 < 0) {
        tolak_akses = 1;
        if (tujuan == 1) {
          saldo1 = saldo1 + harga_fix_palembang;
        }
        if (tujuan == 2) {
          saldo1 = saldo1 + harga_fix_jakarta ;
        }
        if (tujuan == 3) {
          saldo1 = saldo1 + harga_fix_makassar;
        }
      } else {
        lcd.setCursor(0, 1);
        lcd.print(saldo1);
      }
    }
    if (read_rfid == ID_2) {
      if (tujuan == 1) {
        saldo2 = saldo2 - harga_fix_palembang;
      }
      if (tujuan == 2) {
        saldo2 = saldo2 - harga_fix_jakarta ;
      }
      if (tujuan == 3) {
        saldo2 = saldo2 - harga_fix_makassar;
      }
      if (saldo2 < 0) {
        tolak_akses = 1;
        if (tujuan == 1) {
          saldo2 = saldo2 + harga_fix_palembang;
        }
        if (tujuan == 2) {
          saldo2 = saldo2 + harga_fix_jakarta ;
        }
        if (tujuan == 3) {
          saldo2 = saldo2 + harga_fix_makassar;
        }
      } else {
        lcd.setCursor(0, 1);
        lcd.print(saldo2);
      }
    }
    if (read_rfid == ID_3) {
      if (tujuan == 1) {
        saldo3 = saldo3 - harga_fix_palembang;
      }
      if (tujuan == 2) {
        saldo3 = saldo3 - harga_fix_jakarta ;
      }
      if (tujuan == 3) {
        saldo3 = saldo3 - harga_fix_makassar;
      }
      if (saldo3 < 0) {
        tolak_akses = 1;
        if (tujuan == 1) {
          saldo3 = saldo3 + harga_fix_palembang;
        }
        if (tujuan == 2) {
          saldo3 = saldo3 + harga_fix_jakarta ;
        }
        if (tujuan == 3) {
          saldo3 = saldo3 + harga_fix_makassar;
        }
      } else {
        lcd.setCursor(0, 1);
        lcd.print(saldo3);
      }
    }

    if (tolak_akses == 0) {

      digitalWrite(motor, HIGH);
      delay(3000);
      digitalWrite(motor, LOW);
      dorong.write(30);//dorong
    }
    kunci = 3;
    lcd.clear();

  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Saldo Anda");
    if (read_rfid == ID_1) {
      lcd.setCursor(0, 1);
      lcd.print(saldo1);
    }
    if (read_rfid == ID_2) {
      lcd.setCursor(0, 1);
      lcd.print(saldo2);
    }
    if (read_rfid == ID_3) {
      lcd.setCursor(0, 1);
      lcd.print(saldo3);
    }
    delay(2000);
    lcd.clear();

  }




}

void cek_warna() {
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);

  /*Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");
  */
  //sortir.write(150);//hijau
  //sortir.write(80);//biru
  //sortir.write(2);//merah

  if (digitalRead(IR) == LOW) {




    if (r > g and r > b) {
      lcd.setCursor(0, 2);
      lcd.print("Tujuan: Palembang");
      lcd.setCursor(0, 3);
      tujuan = 1;
      harga_palembang = GRAM / 1000 * 6000;
      harga_fix_palembang = harga_palembang;
      Serial.println(harga_fix_palembang);
      sortir.write(2);
    }
    if (b > g and b > r) {
      lcd.setCursor(0, 2);
      lcd.print("Tujuan: Jakarta");
      lcd.setCursor(0, 3);
      tujuan = 2;
      harga_jakarta = GRAM / 1000 * 11000;
      harga_fix_jakarta = harga_jakarta;
      Serial.println(harga_fix_jakarta);
      sortir.write(90);
    }
    if (g > r and g > b) {
      lcd.setCursor(0, 2);
      lcd.print("Tujuan: Makassar");
      tujuan = 3;
      harga_makassar = GRAM / 1000 * 7000;
      harga_fix_makassar = harga_makassar;
      Serial.println(harga_fix_makassar);
      sortir.write(150);
    }
  }



}

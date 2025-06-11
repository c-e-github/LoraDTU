// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024 C-E and others

#include "Configuration.h"
const char* FirmwareVersion = "1.0";
#include <string>
#include "Datastore.h"
#include "Display_Graphic.h"
#include <Hoymiles.h>
#include "InverterSettings.h"
#include "Led_Single.h"
#include "MessageOutput.h"
#include "NetworkSettings.h"
#include "NtpSettings.h"
#include "PinMapping.h"
#include "Scheduler.h"
#include "Utils.h"
#include "WebApi.h"
#include "defaults.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <TaskScheduler.h>
#include <LoRa.h>

SPIClass spiV(VSPI); // VSPI für LoRa verwenden
#define LORA_SCLK_PIN              5
#define LORA_MISO_PIN              19
#define LORA_MOSI_PIN              27
#define LORA_CS_PIN                18
#define LORA_DIO0_PIN              26
#define LORA_RST_PIN               23
std::string ausgabe = "Nr.";
std::string wtproz; // watt in % von 800 watt, vom stromzähler gesendet, plus zähler
std::string wtproz_clean; // watt in % von 800 watt, vom stromzähler gesendet, ohne zähler
std::string recv = ""; // string receiving lora data 
int wattprozent = 0; // watt in % von 800 watt, angepasst für smartmeter.cpp
bool man_aend_wattprozent = false; // watt in % manuell via webseite geändert
bool nureinmal = false;
float dcVoltage_float;
float abschaltwert;
bool hoy_aktiv = false;
int akPower = 0; // aktuell gesetzter wert in % von 800 watt
int NewPower = 0;
int wattmerker = 0;
bool LoraOn = true; // zum ein-/ausschalten der verarbeitung der lora-messages (default: ein)
extern bool man_aend_wattprozent;
uint32_t _lastSmartMeterUpdate = 0;
float  NewV10 = 27.1;
float  NewV20 = 27.2;
float  NewV30 = 27.3;
float  NewV40 = 27.4;
float  NewV50 = 27.6;
float  NewV60 = 27.8;
std::shared_ptr<InverterAbstract> inv;
bool cmdwaitperiod = false;
uint32_t cmdwaitperiodstart;
uint32_t taktstart;
int zaehler = 10; // immer 2-stellig
int zaehl2 = 0; // für abschaltwert-verzögerung
int temp;
bool abschaltwert_belassen = false;
float watts;
char payload[9];
int32_t powerSaldo;
int i = 0;

void setup()
{
   delay(1500);     // When the power is turned on, a delay is required.
   // Initialize serial output
   Serial.begin(SERIAL_BAUDRATE);
   #if ARDUINO_USB_CDC_ON_BOOT
      Serial.setTxTimeoutMs(0);
      delay(100);
   #else
      while (!Serial)
      yield();
   #endif
   MessageOutput.init(scheduler);
   MessageOutput.println();
   MessageOutput.println("Starting LoraDTU");
   delay(1500);     // When the power is turned on, a delay is required.

   // Initialize file system
   MessageOutput.print("Initialize FS... ");
   if (!LittleFS.begin(false)) { // Do not format if mount failed
       MessageOutput.print("failed... trying to format...");
       if (!LittleFS.begin(true)) {
           MessageOutput.print("success");
       } else {
           MessageOutput.print("failed");
       }
   } else {
       MessageOutput.println("done");
   }

   // Read configuration values
   MessageOutput.print("Reading configuration... ");
   if (!Configuration.read()) {
       MessageOutput.print("initializing... ");
       Configuration.init();
       if (Configuration.write()) {
           MessageOutput.print("written... ");
       } else {
           MessageOutput.print("failed... ");
       }
   }
   if (Configuration.get().Cfg.Version != CONFIG_VERSION) {
       MessageOutput.print("migrated... ");
       Configuration.migrate();
   }
   auto& config = Configuration.get();
   MessageOutput.println("done");

   // Load PinMapping
   MessageOutput.print("Reading PinMapping... ");
   if (PinMapping.init(String(Configuration.get().Dev_PinMapping))) {
       MessageOutput.print("found valid mapping ");
   } else {
       MessageOutput.print("using default config ");
   }
   const auto& pin = PinMapping.get();
   MessageOutput.println("done");

   // Initialize WiFi
   MessageOutput.print("Initialize Network... ");
   NetworkSettings.init(scheduler);
   MessageOutput.println("done");
   NetworkSettings.applyConfig();

   // Initialize NTP
   MessageOutput.print("Initialize NTP... ");
   NtpSettings.init();
   MessageOutput.println("done");

   // Initialize WebApi
   MessageOutput.print("Initialize WebApi... ");
   WebApi.init(scheduler);
   MessageOutput.println("done");

   // Initialize Display
   MessageOutput.print("Initialize Display... ");
   Display.init(
      scheduler,
      static_cast<DisplayType_t>(pin.display_type),
      pin.display_data,
      pin.display_clk,
      pin.display_cs,
      pin.display_reset);
   Display.setOrientation(config.Display.Rotation);
   Display.enablePowerSafe = config.Display.PowerSafe;
   Display.enableScreensaver = config.Display.ScreenSaver;
   Display.setContrast(config.Display.Contrast);
   Display.setLanguage(config.Display.Language);
   Display.setStartupDisplay();
   MessageOutput.println("done");

   // Initialize Single LEDs
   MessageOutput.print("Initialize LEDs... ");
   LedSingle.init(scheduler);
   MessageOutput.println("done");

   // Check for default DTU serial
   MessageOutput.print("Check for default DTU serial... ");
   if (config.Dtu.Serial == DTU_SERIAL) {
       MessageOutput.print("generate serial based on ESP chip id: ");
       const uint64_t dtuId = Utils::generateDtuSerial();
       MessageOutput.printf("%0x%08x... ",
           ((uint32_t)((dtuId >> 32) & 0xFFFFFFFF)),
           ((uint32_t)(dtuId & 0xFFFFFFFF)));
       config.Dtu.Serial = dtuId;
       Configuration.write();
   }
   MessageOutput.println("done");

   // lora
   spiV.begin(LORA_SCLK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN);
   delay(1500);
   pinMode(spiV.pinSS(), OUTPUT); //HSPI SS 
   LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
   if (!LoRa.begin(868E6)) {
      Serial.println("Starting LoRa failed!");
      delay(5000);
   } else {      
      LoRa.setSpreadingFactor(6);  // Supported values are between `6` and `12`. defaults to `7`
      // If a spreading factor of `6` is set, implicit header mode must be used to transmit and receive packets.
      LoRa.setCodingRate4(5); // default `5`, Supported values `5` - `8`, correspond to coding rates of `4/5` and `4/8
      LoRa.disableCrc();  // by default a CRC is not used
      // LoRa.parsePacket(9); // in der loop, aktiviert Implicit Header Mode mit fixer Länge
      LoRa.setPreambleLength(8); // default 8, Supported values are between `6` and `65535`
      LoRa.setSyncWord(0x12); // byte value to use as the sync word, defaults to `0x12`
      LoRa.setSignalBandwidth(125E3);  // signal bandwidth in Hz, defaults to `125E3`
      // Supported values are `7.8E3`, `10.4E3`, `15.6E3`, `20.8E3`, `31.25E3`, `41.7E3`, `62.5E3`, `125E3`, `250E3`, and `500E3`.
      Serial.println("LoRa was started!");
      delay(1000);
   }

   // pin 2 für display-ein-aus-taster
   pinMode(2, INPUT_PULLUP);

   InverterSettings.init(scheduler);
   Datastore.init(scheduler);
}

int func_Wattanpassung(int temp) {
    if (temp > 30) {
        return temp * 0.7;
    } else if (temp > 20) {
        return temp * 0.8;
    } else if (temp > 10) {
        return temp * 0.88;
    } else if (temp > 5) {
        return temp * 0.95;
    }
    return temp; // Keine Änderung, falls temp <= 5
}


void loop()
{
   scheduler.execute();

   if (nureinmal == false){
      inv = Hoymiles.getInverterByPos(0);
      if (inv){
          nureinmal = true;
      }
   }

   int packetSize = LoRa.parsePacket(9); // aktiviert Implicit Header Mode mit fixer Länge
   if (packetSize == 9) { // received a packet
      MessageOutput.printf("XXXXXXXXXX packet received XXXXXXXXXXXXXXXXXX  ");

      i = 0;
      while (LoRa.available() && i < 9) {
         payload[i++] = LoRa.read();
      }

      //MessageOutput.printf("\n");
      //for (int i = 0; i < 9; i++) {
      //   MessageOutput.printf("%02X ", payload[i]);
      //}
      //MessageOutput.printf("\n");
      //MessageOutput.printf("\n");


      if (memcmp(payload, "abcdef", 6) == 0) {       // Vergleiche Identifier
         //#################### gültige message erhalten ########################################
         // Rekonstruiere powerSaldo (signed int32 aus 3 Bytes)
         powerSaldo = ((int32_t)payload[6] << 16) | (payload[7] << 8) | payload[8];
         if (powerSaldo & 0x800000) { // Falls negativ: prüfe höchstes Bit im 24-Bit-Wert
            powerSaldo |= 0xFF000000;  // Sign extension
         }
         Serial.print("powerSaldo: ");
         Serial.println(powerSaldo);
         MessageOutput.printf("Lora-Empfang: %d W\r\n", powerSaldo);
         wattprozent = powerSaldo / 8;
         if (wattprozent < -99){
            wattprozent = -99;
         }
         if (wattprozent > 99){
            wattprozent = 99;
         }

         zaehler++; // damit wattanzeige jedesmal aufblinkt
         if(zaehler > 99){
            zaehler = 10;
         }

         wtproz = std::to_string(wattprozent);
         if (wattprozent == 0){
            wtproz_clean = "+00";
         }
         if (wattprozent > 0 && wattprozent < 10){
            wtproz_clean = "+0";
            wtproz_clean += wtproz;
         }
         if (wattprozent == 10 || wattprozent > 10){
            wtproz_clean = "+";
            wtproz_clean += wtproz;
         }
         if (wattprozent > -10 && wattprozent < 0){
            wtproz_clean = "-0";
            wtproz_clean += wtproz[wtproz.length()-1]; // letztes zeichen in wtproz (also der einstellige wert ohne minuszeichen)
         }
         if (wattprozent < -9){
            wtproz_clean = wtproz;
         }
         wtproz = wtproz_clean + std::to_string(zaehler); // immer 3 stellen + zähler

         MessageOutput.print("Wtproz_clean: ");
         MessageOutput.print(wtproz_clean.c_str());
         temp = atoi(wtproz_clean.c_str());
         wattprozent = func_Wattanpassung(temp);
         MessageOutput.printf("  Wattprozent (angepasst): %d %%\r\n", wattprozent);

         if (inv){
            dcVoltage_float = inv->Statistics()->getChannelFieldValue(TYPE_DC, CH0, FLD_UDC);
         }
         MessageOutput.printf("Batteriespannung: %f V\r\n", dcVoltage_float);
         watts = Datastore.getTotalAcPowerEnabled();
         if (watts > 0){
            hoy_aktiv = true;
         } else {
            hoy_aktiv = false;
         }

         if (hoy_aktiv == false){
            MessageOutput.printf("\r\n");
            MessageOutput.printf("hoy_aktiv == false  ");
            akPower = 0;
            wattmerker = 0;
         } else{
            MessageOutput.printf("\r\n");
            MessageOutput.printf("hoy_aktiv == true  ");
         }
      einsprung_man_aend:
         // damit manuelle leistungänderung korrekt umgesetzt wird ("wattprozent" wurde in WebApi_limit.cpp manuell gesetzt)
         if (man_aend_wattprozent == true){
            MessageOutput.printf("man_aend_wattprozent == true\r\n");
            delay(500);
            akPower = 0;
         };

         NewPower = akPower + wattprozent;
         wattmerker = wattprozent;
         wattprozent = 0; // damit beim nächsten durchlauf ohne explizites neusetzen von wattprozent nix geändert wird

         if (NewPower < 2){
            NewPower = 2;
         }
         if (NewPower > 60){ 
            NewPower = 60; // max. 480 Watt
         }
         MessageOutput.printf("akPower: %d %%  ", akPower);
         MessageOutput.printf("NewPower: %d %%\r\n", NewPower);
         MessageOutput.printf("\r\n");

         if (dcVoltage_float < 26.9){
            NewV10 = 27.1;
         }
         if (dcVoltage_float > NewV10 && NewPower < 10){
            NewPower = 10; // einspeisung bei voller batterie
            NewV10 = 26.9;
         }
         if (dcVoltage_float < 27.0){
            NewV20 = 27.2;
         }
         if (dcVoltage_float > NewV20 && NewPower < 20){
            NewPower = 20; // einspeisung bei voller batterie
            NewV20 = 27.0;
         }
         if (dcVoltage_float < 27.1){
            NewV30 = 27.3;
         }
         if (dcVoltage_float > NewV30 && NewPower < 30){
            NewPower = 30; // einspeisung bei voller batterie
            NewV30 = 27.1;
         }
         if (dcVoltage_float < 27.2){
            NewV40 = 27.4;
         }
         if (dcVoltage_float > NewV40 && NewPower < 40){
            NewPower = 40; // einspeisung bei voller batterie
            NewV40 = 27.2;
         }
         if (dcVoltage_float < 27.3){
            NewV50 = 27.6;
         }
         if (dcVoltage_float > NewV50 && NewPower < 50){
            NewPower = 50; // einspeisung bei voller batterie
            NewV50 = 27.3;
         }
            if (dcVoltage_float < 27.5){
            NewV60 = 27.8;
         }
         if (dcVoltage_float > NewV60 && NewPower < 60){
            NewPower = 60; // einspeisung bei voller batterie
            NewV60 = 27.5;
         }
      } else {
         Serial.println("verstümmeltes Paket oder Paket nicht für mich !!!");
      }   
      if ((NewPower != akPower) && (hoy_aktiv == true)){
         if ((LoraOn == true) || ((LoraOn == false) && (man_aend_wattprozent == true))){
            inv->sendActivePowerControlRequest(static_cast<float>(NewPower), PowerLimitControlType::RelativNonPersistent);
            MessageOutput.printf("###################### Wattänderung ########################\r\n");
            if(akPower - NewPower > 20){ // starke reduktion der leistung
               abschaltwert_belassen = true;
            }
            akPower = NewPower;
         }
      }
      man_aend_wattprozent = false;  
      _lastSmartMeterUpdate = millis();
   }
   // received a packet ENDE
   //################################## Hoy_Switch ##################################################
   if (cmdwaitperiod == true){
      if ((millis() - cmdwaitperiodstart) > (6000)) { // wartezeit nach ein-/ausschalten nach 6 sekunden beenden
         cmdwaitperiod = false;
      }
   }

   if (cmdwaitperiod == false){
      if ((millis() - taktstart) < (2000)) { // 2-sekunden-takt für den nachfolgenden code
         goto no_takt;
      }
      taktstart = millis();
      if (inv){
         dcVoltage_float = inv->Statistics()->getChannelFieldValue(TYPE_DC, CH0, FLD_UDC);
      }
      watts = Datastore.getTotalAcPowerEnabled();
      if (watts > 0){
         hoy_aktiv = true;
      } else {
         hoy_aktiv = false;
      }

      if (hoy_aktiv == false){
         akPower = 0;
         wattmerker = 0;
      }

      if (man_aend_wattprozent == true){
         MessageOutput.printf("sprung zu einsprung_man_aend ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
         MessageOutput.printf("sprung zu einsprung_man_aend ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
         MessageOutput.printf("sprung zu einsprung_man_aend ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
         delay(1000);
         goto einsprung_man_aend;
      };


      // ---------------------------abschaltwerte -----------------------------------
      if (abschaltwert_belassen == true){
         zaehl2++;
         MessageOutput.printf("zaehl2: %d \r\n", zaehl2);
         if(zaehl2 > 20){
            abschaltwert_belassen = false;
            zaehl2 = 0;
         }
         goto sprung1; // damit abschaltwert zunächst unverändert bleibt 
      }

      abschaltwert = (-0.00067*NewPower*NewPower) - (0.0043*NewPower) + 25.4; // ergibt werte zwischen 25.4 und 23.0
      if (abschaltwert < 23.0){
         abschaltwert = 23.0;
      }

      sprung1:
      MessageOutput.printf("abschaltwert: %f V\r\n", abschaltwert);
      if (dcVoltage_float > 26.2 && (hoy_aktiv == false) && NewPower > 0 && (LoraOn == true)){
         inv->sendPowerControlRequest(1); // hoymiles einschalten
         MessageOutput.printf("########################## Einschaltbefehl ########################\r\n");
         hoy_aktiv = true;
         cmdwaitperiod = true;
         cmdwaitperiodstart = millis();
      }

      if ((dcVoltage_float < abschaltwert) && hoy_aktiv){
         inv->sendPowerControlRequest(0); // hoymiles ausschalten
         MessageOutput.printf("!!!!!!!!!!!!!!!!!!! Abschaltwert-Ausschaltung !!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
         hoy_aktiv = false;
         akPower = 0;
         wattmerker = 0;
         delay(1000);
         cmdwaitperiod = true;
         cmdwaitperiodstart = millis();
      }

      if (dcVoltage_float < 23.0){  // notfall-ausschaltung zum batterieschutz
         // sorgt auch für initiale ausschaltung beim starten der LoraDTU
         inv->sendPowerControlRequest(0); // hoymiles ausschalten
         MessageOutput.printf("!!!!!!!!!!!!!!!!!!!!! Batterieschutz-Ausschaltung !!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
         hoy_aktiv = false;
         akPower = 0;
         wattmerker = 0;
         cmdwaitperiod = true;
         cmdwaitperiodstart = millis();
      }
   no_takt:
   taktstart = taktstart; // weil nach sprungmarke code kommen muss
   }
}





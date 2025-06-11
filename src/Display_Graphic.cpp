// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024 C-E and others
 
#include "Display_Graphic.h"
#include "Datastore.h"
#include <NetworkSettings.h>
#include <map>
#include <time.h>
#include <Hoymiles.h>
#include "MessageOutput.h"
std::string tagmerker;
int yielddayoffset = 0;
int yielddaych0offset = 0;
int yielddaych1offset = 0;
float yieldyesterday = 0;
bool statusTaster = HIGH;         // aktueller Status des Taster an Pin 2
bool statusTasterLetzter = HIGH;  // vorheriger Status des Tasters an Pin 2
bool displayPowerSave = false;    // bedeutet Display ist an
extern int NewPower;
unsigned long lastmillis;

std::map<DisplayType_t, std::function<U8G2*(uint8_t, uint8_t, uint8_t, uint8_t)>> display_types = {
    { DisplayType_t::PCD8544, [](uint8_t reset, uint8_t clock, uint8_t data, uint8_t cs) { return new U8G2_PCD8544_84X48_F_4W_HW_SPI(U8G2_R0, cs, data, reset); } },
    { DisplayType_t::SSD1306, [](uint8_t reset, uint8_t clock, uint8_t data, uint8_t cs) { return new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, reset, clock, data); } },
    { DisplayType_t::SH1106, [](uint8_t reset, uint8_t clock, uint8_t data, uint8_t cs) { return new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, reset, clock, data); } },
};

// Language defintion, respect order in languages[] and translation lists
#define I18N_LOCALE_EN 0
#define I18N_LOCALE_DE 1
#define I18N_LOCALE_FR 2

// Languages supported. Note: the order is important and must match locale_translations.h
const uint8_t languages[] = {
    I18N_LOCALE_EN,
    I18N_LOCALE_DE,
    I18N_LOCALE_FR
};

static const char* const i18n_offline[] = { "Offline", "Offline", "Offline" };
static const char* const i18n_current_power_w[] = { "%3.1f W", "%3.1f W", "%3.1f W" }; 
static const char* const i18n_current_power_kw[] = { "%2.1f kW", "%2.1f kW", "%2.1f kW" };
static const char* const i18n_yield_today_wh[] = { "today: %4.0f Wh", "Heute: %4.0f Wh", "auj.: %4.0f Wh" };
static const char* const i18n_yield_total_kwh[] = { "total: %.1f kWh", "Ges.: %.1f kWh", "total: %.1f kWh" };
static const char* const i18n_date_format[] = { "%m/%d/%Y %H:%M", "%d.%m.%Y %H:%M", "%d/%m/%Y %H:%M" };

DisplayGraphicClass::DisplayGraphicClass()
{
}

DisplayGraphicClass::~DisplayGraphicClass()
{
    delete _display;
}

void DisplayGraphicClass::init(Scheduler& scheduler, const DisplayType_t type, const uint8_t data, const uint8_t clk, const uint8_t cs, const uint8_t reset)
{
    _display_type = type;
    if (_display_type > DisplayType_t::None) {
        auto constructor = display_types[_display_type];
        _display = constructor(reset, clk, data, cs);
        _display->begin();
        setContrast(DISPLAY_CONTRAST);
        setStatus(true);
    }

    scheduler.addTask(_loopTask);
    _loopTask.setCallback(std::bind(&DisplayGraphicClass::loop, this));
    _loopTask.setIterations(TASK_FOREVER);
    _loopTask.setInterval(_period);
    _loopTask.enable();
}

void DisplayGraphicClass::calcLineHeights()
{
    uint8_t yOff = 0;
    for (uint8_t i = 0; i < 4; i++) {
        setFont(i);
        yOff += (_display->getMaxCharHeight());
        _lineOffsets[i] = yOff;
    }
}

void DisplayGraphicClass::setFont(const uint8_t line)
{
    switch (line) {
    case 0:
        _display->setFont((_isLarge) ? u8g2_font_ncenB14_tr : u8g2_font_logisoso16_tr);
        break;
    case 3:
        _display->setFont(u8g2_font_6x10_tr);
        break;
    default:
        _display->setFont((_isLarge) ? u8g2_font_ncenB10_tr : u8g2_font_5x8_tr);
        break;
    }
}

void DisplayGraphicClass::printText(const char* text, const uint8_t line)
{
    uint8_t dispX;
    if (!_isLarge) {
        dispX = (line == 0) ? 5 : 0;
    } else {
        dispX = (line == 0) ? 10 : 5;
    }
    setFont(line);

    dispX += enableScreensaver ? (_mExtra % 7) : 0;
    _display->drawStr(dispX, _lineOffsets[line], text);
}

void DisplayGraphicClass::setOrientation(const uint8_t rotation)
{
    if (_display_type == DisplayType_t::None) {
        return;
    }

    switch (rotation) {
    case 0:
        _display->setDisplayRotation(U8G2_R0);
        break;
    case 1:
        _display->setDisplayRotation(U8G2_R1);
        break;
    case 2:
        _display->setDisplayRotation(U8G2_R2);
        break;
    case 3:
        _display->setDisplayRotation(U8G2_R3);
        break;
    }

    _isLarge = (_display->getWidth() > 100);
    calcLineHeights();
}

void DisplayGraphicClass::setLanguage(const uint8_t language)
{
    _display_language = language < sizeof(languages) / sizeof(languages[0]) ? language : DISPLAY_LANGUAGE;
}

void DisplayGraphicClass::setStartupDisplay()
{
    if (_display_type == DisplayType_t::None) {
        return;
    }

    _display->clearBuffer();
    printText("LoraDTU", 0);
    _display->sendBuffer();
}

void DisplayGraphicClass::loop()
{
   // Taster abfragen, wenn Taster gedrückt wurde, Display ein oder ausschalten
   statusTaster = digitalRead(2); 
   if (statusTaster == !statusTasterLetzter) {// Wenn aktueller Tasterstatus anders ist als der letzte Tasterstatus
      if (statusTaster == LOW) {// Wenn Taster gedrückt
         if (displayPowerSave == true){ // Display an- bzw. ausschalten
            displayPowerSave = false;
         } else {
            displayPowerSave = true;
         } 
      }            
   }
   statusTasterLetzter = statusTaster; // merken des letzten Tasterstatus
 
    if (_display_type == DisplayType_t::None) {
        return;
    }

    _loopTask.setInterval(_period);

    _display->clearBuffer();

    //=====> Actual Production ==========
    String dcVoltage;
    if (Datastore.getIsAtLeastOneReachable()) {
       // anzeige spannung dc-eingang 
       auto inv = Hoymiles.getInverterByPos(0);
       dcVoltage = inv->Statistics()->getChannelFieldValueString(TYPE_DC, CH0, FLD_UDC);
       dcVoltage += " V";
       if (Datastore.getIsAllEnabledProducing()) {
          const float watts = Datastore.getTotalAcPowerEnabled();
          if (watts > 999) {
              snprintf(_fmtText, sizeof(_fmtText), i18n_current_power_kw[_display_language], watts / 1000);
          } else {
              snprintf(_fmtText, sizeof(_fmtText), i18n_current_power_w[_display_language], watts);
          }
          printText(_fmtText, 0);
          printText(dcVoltage.c_str(), 1);
       }
        _previousMillis = millis();
    }

    //=====> An aber Offline ===========
    if (Datastore.getIsAtLeastOneReachable() &&!Datastore.getIsAllEnabledProducing()) {
        printText(i18n_offline[_display_language], 0);
        printText(dcVoltage.c_str(), 1);
        // check if it's time to enter power saving mode
        if (millis() - _previousMillis >= (_interval * 2)) { 
            displayPowerSave = enablePowerSafe;
        }
    }
    //<=======================

    //=====> IP or Date-Time ========
    if (!(_mExtra % 10) && NetworkSettings.localIP()) {
        printText(NetworkSettings.localIP().toString().c_str(), 2); 
    } else {
        // Get current date & time
        time_t now = time(nullptr);
        strftime(_fmtText, sizeof(_fmtText), i18n_date_format[_display_language], localtime(&now));
        printText(_fmtText, 2); 
        // vorhandene ausbeute ermitteln, damit sie bei tagesausbeute abgezogen werden kann 
        std::string temp(_fmtText);

        if(Datastore.getTotalAcYieldDayEnabled() > 0){
           if (tagmerker != temp.substr(0, 10)){ // tageswechsel
              if (yielddayoffset > 0){
                 yieldyesterday = Datastore.getTotalAcYieldDayEnabled() - yielddayoffset;   
              }
              yielddayoffset = Datastore.getTotalAcYieldDayEnabled();
              // tageswerte string 1 und 2
              auto inv = Hoymiles.getInverterByPos(0);
              yielddaych0offset = inv->Statistics()->getChannelFieldValue(TYPE_DC, CH0, FLD_YD);
              yielddaych1offset = inv->Statistics()->getChannelFieldValue(TYPE_DC, CH1, FLD_YD);

              tagmerker = temp.substr(0, 10);
              Serial.print("XXXX tageswechsel XXXX: ");
              Serial.println(yielddayoffset);
           }
        }
    }
    
    std::string temp2 = "Setpoint: " + std::to_string(NewPower) + " %";
    printText(temp2.c_str(), 3);

    _display->sendBuffer();

    _mExtra++;

    if (!_displayTurnedOn) {
        displayPowerSave = true;
    }

    _display->setPowerSave(displayPowerSave);
}

void DisplayGraphicClass::setContrast(const uint8_t contrast)
{
    if (_display_type == DisplayType_t::None) {
        return;
    }
    _display->setContrast(contrast * 2.55f);
}

void DisplayGraphicClass::setStatus(const bool turnOn)
{
    _displayTurnedOn = turnOn;
}

DisplayGraphicClass Display;

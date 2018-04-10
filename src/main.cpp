#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include "RTClib.h"
#include "Adafruit_GFX.h"
#include "Adafruit_NeoPixel.h"
#include "Adafruit_LEDBackpack.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define BAUD_RATE 57600

struct RGB {
  byte R;
  byte G;
  byte B;
};

void initButtons();
void initLeds();
void initRGBPixels();
void initLedDisplay();
void initRTC();
void initMp3Player();
void setDefaultRTCTime();
int calculateTime();
void readButtonStates();
void drawTimeOnDisplay(int time, bool blinkColon, bool showColon, bool showAlarmActive);
int retrieveSettingFromMemory(int address, int minValue, int maxValue, int defaultValue);
void resetLights();
void resetDisplay();
void enableAlarm();
void setLight(byte lightNr, RGB rgb, int delayTime);
void muteAlarm();
void enableAlarm();
void storeSettingInMemory(int address, int value);
void handleMenu(byte type);
void setLights(RGB rgb, int mode);

Adafruit_7segment ledDisplay;
RTC_DS1307 rtc;
SoftwareSerial mp3Serial(10, 11);
DFRobotDFPlayerMini mp3Player;
Adafruit_NeoPixel rgbPixels;

struct ButtonStates {
  byte functionButtonState;
  byte plusButtonState;
  byte minButtonState;
  byte snoozeButtonState;
};
ButtonStates buttonStates = { LOW, LOW, LOW, LOW };

struct Settings {
  int volumeLevel;
  int activeSound;
  int ledBrightness;
  bool blinkColon;
  int rgbPixelBrigthness;
  int activeLedColor;
  int ledMode;
  int alarmHour;
  int alarmMinute;
  int alarm;
};

struct SettingValue {
  int currentValue;
  int minValue;
  int maxValue;
};

struct DrawSettings {
  byte style;
  bool blink;
  bool showColon;
  int delay;
  byte menuIndex;
  bool showMenuIndex;
};

const bool DEBUG = false;
const byte FUNCTION_BUTTON = 2;
const byte PLUS_BUTTON = 3;
const byte MIN_BUTTON = 4;
const byte SNOOZE_BUTTON = 5;
const byte SNOOZE_LED = 6;
const byte RGB_LED = 7;
const byte RGB_PIXELS = 2;

const int DELAY_FAST = 500;
const int DELAY_NORMAL = 1000;
const int DELAY_SLOW = 1500;
const int LED_ON_OFF_DELAY_FAST = 100;
const int LED_ON_OFF_DELAY_NORMAL = 200;
const int LED_ON_OFF_DELAY_SLOW = 300;

const int SET_ALARM_DELAY = 10000;

const byte VOLUME = 20;
const byte MIN_VOLUME = 0;
const byte MAX_VOLUME = 30;

const byte MODE_CLOCK = 0;
const byte MODE_MENU = 1;

const byte DRAW_LEFT_NUMBERS_STYLE = 0;
const byte DRAW_RIGHT_NUMBERS_STYLE = 1;
const byte DRAW_ALL_NUMBERS_STYLE = 2;

const byte LED_SEGMENT_FIRST_NUMBER = 5;
const byte LED_SEGMENT_SECOND_NUMBER = 3;
const byte LED_SEGMENT_THIRD_NUMBER = 1;
const byte LED_SEGMENT_FORTH_NUMBER = 0;
const byte LED_BRIGHTNESS_MIN = 1;
const byte LED_BRIGHTNESS_MAX = 15;
const byte RGB_PIXEL_BRIGTHNESS_MIN = 0;
const int RGB_PIXEL_BRIGTHNESS_MAX = 255;
const byte LED_MODE_MIN = 1;
const int LED_MODE_MAX = 8;

const byte RGB_PIXEL_MODE_BLINK_SLOW= 1;
const byte RGB_PIXEL_MODE_BLINK_NORMAL= 2;
const byte RGB_PIXEL_MODE_BLINK_FAST= 3;
const byte RGB_PIXEL_MODE_ON= 4;
const byte RGB_PIXEL_MODE_BLINK_ON_OFF_SLOW= 5;
const byte RGB_PIXEL_MODE_BLINK_ON_OFF_NORMAL= 6;
const byte RGB_PIXEL_MODE_BLINK_ON_OFF_FAST= 7;
const byte RGB_PIXEL_MODE_OFF= 8;

const byte SOUND_LEVEL_MENU = 1;
const byte ALARM_SOUND_MENU = 2;
const byte DISPLAY_BRIGHTNESS_LEVEL_MENU = 3;
const byte DISPLAY_COLON_MENU = 4;
const byte RGB_PIXEL_BRIGTHNESS_LEVEL_MENU = 5;
const byte RGB_PIXEL_COLOR_MENU = 6;
const byte RGB_PIXEL_MODE_MENU = 7;
const byte TIME_HOUR_MENU = 8;
const byte TIME_MINUTE_MENU = 9;
const byte ALARM_TIME_HOUR_MENU = 10;
const byte ALARM_TIME_MINUTE_MENU = 11;

const byte MEM_ADDRESS_SOUND_LEVEL = 0;
const byte MEM_ADDRESS_ALARM_SOUND = 1;
const byte MEM_DISPLAY_BRIGHTNESS_LEVEL = 2;
const byte MEM_BLINK_COLON = 3;
const byte MEM_RGB_PIXEL_BRIGHTNESS_LEVEL = 4;
const byte MEM_RGB_PIXEL_COLOR = 5;
const byte MEM_RGB_PIXEL_MODE = 6;
const byte MEM_ALARM = 7;
const byte MEM_ALARM_HOUR = 8;
const byte MEM_ALARM_MINUTE = 9;

// Pre defined colors:
const RGB Black = { 0,0,0 };
const RGB White = { 255,255,255 };
const RGB Red = { 255,0,0 };
const RGB Lime = { 0,255,0 };
const RGB Blue = { 0,0,255 };
const RGB Yellow = { 255,255,0 };
const RGB Purple = { 128,0,128 };
const RGB Orange = { 255,165,0 };
const RGB GoldenRod = { 218, 165, 32 };
const RGB Firebrick = { 178, 34,34 };
const RGB Green = { 0, 128, 0};
const RGB Aqua = { 0, 255, 255 };
const RGB LightSlateGray = { 119, 136, 153 };
const RGB WhiteSmoke = { 245, 245, 245 };
const RGB OrangeRed = { 255, 69, 0 };
const RGB DarkKhaki = { 189, 183, 107 };
const RGB DarkRed = { 139, 0, 0 };
const RGB Navy = { 0, 0, 128 };
const RGB Teal = {0, 128, 128 };
const RGB Magenta = { 255, 0, 255 };
const RGB LightSkyBlue = { 135, 206, 250 };
const RGB SlateBlue = { 106, 90, 205 };

RGB LedColors[] = { Black, White, Red, Lime, Blue, Yellow, Purple, Orange, GoldenRod, Firebrick, Green, Aqua, LightSlateGray, WhiteSmoke, OrangeRed, DarkKhaki, DarkRed, Navy, Teal, Magenta, LightSkyBlue, SlateBlue};

int alarm = 8;
int playAlarm = false;
bool snooze = false;
byte activeMode;
byte activeSettingMenu;
bool showTimeColon;
bool snoozeLedOn = true;
bool ledOn = true;
bool nightLightOn = false;
bool triggerNightLight = false;
bool triggerSnooze = false;
bool alarmActive = false;
Settings settings;
int newHour = -1;
int newMinute = -1;
int newAlarmHour = -1;
int newAlarmMinute = -1;

unsigned long previousDrawTime = 0;
unsigned long previousLedTime = 0;
unsigned long previousSetHourTime = 0;
unsigned long previousSnoozeLedTime = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println(F("Starting alarm clock..."));

  initLedDisplay();
  initRTC();
  initMp3Player();
  initButtons();
  initLeds();
  initRGBPixels();

  activeMode = MODE_CLOCK;
  activeSettingMenu = SOUND_LEVEL_MENU;
  nightLightOn = false;

  Serial.println(F("Alarm clock ready."));
}

void initButtons() {
  Serial.println(F("Initializing buttons..."));

  pinMode(FUNCTION_BUTTON, INPUT_PULLUP);
  pinMode(PLUS_BUTTON, INPUT_PULLUP);
  pinMode(MIN_BUTTON, INPUT_PULLUP);
  pinMode(SNOOZE_BUTTON, INPUT_PULLUP);
}

void initLeds() {
  Serial.println(F("Initializing leds..."));

  pinMode(SNOOZE_LED, OUTPUT);
}

void initRGBPixels() {
  Serial.println(F("Initializing RGB pixels..."));

  rgbPixels = Adafruit_NeoPixel(RGB_PIXELS, RGB_LED, NEO_RGB);
  settings.rgbPixelBrigthness = retrieveSettingFromMemory(MEM_RGB_PIXEL_BRIGHTNESS_LEVEL, RGB_PIXEL_BRIGTHNESS_MIN, RGB_PIXEL_BRIGTHNESS_MAX, RGB_PIXEL_BRIGTHNESS_MAX);
  settings.activeLedColor = retrieveSettingFromMemory(MEM_RGB_PIXEL_COLOR, 0, (sizeof(LedColors) / sizeof(RGB) -1 ) , 1);
  settings.ledMode = retrieveSettingFromMemory(MEM_RGB_PIXEL_MODE, LED_MODE_MIN, LED_MODE_MAX, LED_MODE_MIN);

  rgbPixels.setBrightness(settings.rgbPixelBrigthness);
  rgbPixels.begin();

  resetLights();
}

void initLedDisplay() {
  Serial.println(F("Initializing LED display..."));

  ledDisplay = Adafruit_7segment();
  showTimeColon = true;

  settings.ledBrightness = retrieveSettingFromMemory(MEM_DISPLAY_BRIGHTNESS_LEVEL, LED_BRIGHTNESS_MIN, LED_BRIGHTNESS_MAX, LED_BRIGHTNESS_MAX);
  settings.blinkColon = retrieveSettingFromMemory(MEM_BLINK_COLON, 0, 1, 1);

  ledDisplay.setBrightness(settings.ledBrightness);
  ledDisplay.begin(0x70);
  resetDisplay();
}

void initRTC() {
  Serial.println(F("Initializing RTC..."));

  settings.alarm = retrieveSettingFromMemory(MEM_ALARM, 0, 2359, 0);
  settings.alarmHour = retrieveSettingFromMemory(MEM_ALARM_HOUR, 0, 23, 0);
  settings.alarmMinute = retrieveSettingFromMemory(MEM_ALARM_MINUTE, 0, 59, 0);

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC."));
    while (true);
  }

  if (!rtc.isrunning()) {
    setDefaultRTCTime();
  }
}

void initMp3Player() {
  Serial.println(F("Initializing MP3Player..."));

  mp3Serial.begin(9600);

  if (!mp3Player.begin(mp3Serial)) {
    Serial.println(F("Unable to start MP3Player."));
    while (true);
  }

  settings.volumeLevel = retrieveSettingFromMemory(MEM_ADDRESS_SOUND_LEVEL, MIN_VOLUME, MAX_VOLUME, VOLUME);
  settings.activeSound = retrieveSettingFromMemory(MEM_ADDRESS_ALARM_SOUND, 1, 14, 1);

  mp3Player.setTimeOut(500);
  mp3Player.volume(settings.volumeLevel);
  mp3Player.EQ(DFPLAYER_EQ_NORMAL);
  mp3Player.outputDevice(DFPLAYER_DEVICE_SD);
}

void setDefaultRTCTime() {
  Serial.println(F("Initalize RTC with default time."));
  rtc.adjust(DateTime(0, 0, 0, 0, 0, 0));
}

void loop() {
  int currentTime = calculateTime();
  unsigned long currentMillis = millis();

  readButtonStates();

  if (activeMode == MODE_CLOCK) {
    // Draw time every second
    if (currentMillis - previousDrawTime >= 1000) {
      drawTimeOnDisplay(currentTime, settings.blinkColon, showTimeColon, alarmActive);
      // Blink the colon to indicate seconds.
      showTimeColon = !showTimeColon;

      previousDrawTime = currentMillis;
    }

    if (alarmActive && (currentTime == settings.alarm) && !playAlarm) {
       Serial.println(F("Sound alarm"));
       enableAlarm();
    }

    if (buttonStates.minButtonState == LOW && !triggerNightLight) {
      Serial.println(F("Trigger night light"));
      nightLightOn = !nightLightOn;
      triggerNightLight = true;
    }

    if (buttonStates.minButtonState == HIGH) {
      triggerNightLight = false;
    }

    if (buttonStates.minButtonState == HIGH) {
      triggerSnooze = false;
    }

    // Activate alarm after 5 secondes if plus button is presses in clock mode.
    if (buttonStates.plusButtonState == LOW  && !triggerSnooze) {
      alarmActive = !alarmActive;
      triggerSnooze = !triggerSnooze;

      Serial.println(F("Trigger alarm"));

      if (alarmActive) {
        setLight(0, Green, 0);
        setLight(1, Green, 0);
      }
      else {
        setLight(0, Red, 0);
        setLight(1, Red, 0);
        digitalWrite(SNOOZE_LED, LOW);
      }

      delay(1500);
    }

    if (buttonStates.snoozeButtonState == LOW && alarmActive && playAlarm) {
      Serial.println(F("Mute alarm"));
      muteAlarm();
    }

    // First show alarm else nightlight
    if (alarmActive && playAlarm) {
      // blink snooze button
      if (currentMillis - previousSnoozeLedTime >= 500) {
        previousSnoozeLedTime = currentMillis;
        snoozeLedOn = !snoozeLedOn;
        digitalWrite(SNOOZE_LED, snoozeLedOn);
      }

      setLights(LedColors[settings.activeLedColor], settings.ledMode);
    }
    else if (nightLightOn) {
      setLights(LedColors[settings.activeLedColor], RGB_PIXEL_MODE_ON);
    }
    else {
      setLights(LedColors[settings.activeLedColor], RGB_PIXEL_MODE_OFF);
    }
  }

  if (activeMode == MODE_MENU) {
    handleMenu(activeSettingMenu);
  }

  // Goto next menu
  if (buttonStates.functionButtonState == LOW && buttonStates.snoozeButtonState == LOW && activeMode == MODE_MENU) {
    activeSettingMenu++;
    if (activeSettingMenu > ALARM_TIME_MINUTE_MENU) {
      activeSettingMenu = SOUND_LEVEL_MENU;
    }

    delay(100);
  }

  if (buttonStates.functionButtonState == LOW && activeMode != MODE_MENU) {
    activeMode = MODE_MENU;
  }
  else if(buttonStates.functionButtonState == HIGH && activeMode == MODE_MENU) {
    Serial.println(F("Store settings"));
    activeMode = MODE_CLOCK;

    storeSettingInMemory(MEM_ADDRESS_SOUND_LEVEL, settings.volumeLevel);
    storeSettingInMemory(MEM_ADDRESS_ALARM_SOUND, settings.activeSound);
    storeSettingInMemory(MEM_DISPLAY_BRIGHTNESS_LEVEL, settings.ledBrightness);
    storeSettingInMemory(MEM_BLINK_COLON, settings.blinkColon);
    storeSettingInMemory(MEM_RGB_PIXEL_BRIGHTNESS_LEVEL, settings.rgbPixelBrigthness);
    storeSettingInMemory(MEM_RGB_PIXEL_COLOR, settings.activeLedColor);
    storeSettingInMemory(MEM_RGB_PIXEL_MODE, settings.ledMode);

    settings.alarm = (settings.alarmHour * 100) + settings.alarmMinute;
    storeSettingInMemory(MEM_ALARM, settings.alarm);
    storeSettingInMemory(MEM_ALARM_HOUR, settings.alarmHour);
    storeSettingInMemory(MEM_ALARM_MINUTE, settings.alarmMinute);

    if (newHour > -1) {
       if (newMinute == -1) {
         newMinute = rtc.now().minute();
       }

       rtc.adjust(DateTime(0, 0, 0, newHour, newMinute, 0));
    }

    activeSettingMenu = SOUND_LEVEL_MENU;
    newHour = -1;
    newMinute = -1;
  }
}

int calculateTime() {
  // Get the current time and convert it to an integer number.
  DateTime now = rtc.now();
  int hour = now.hour() * 100;
  int minute = now.minute();

  return hour + minute;
}

void readButtonStates() {
  buttonStates.functionButtonState = digitalRead(FUNCTION_BUTTON);
  buttonStates.plusButtonState = digitalRead(PLUS_BUTTON);
  buttonStates.minButtonState = digitalRead(MIN_BUTTON);
  buttonStates.snoozeButtonState = digitalRead(SNOOZE_BUTTON);

  if (DEBUG) {
    Serial.print(buttonStates.functionButtonState);
    Serial.print(buttonStates.plusButtonState);
    Serial.print(buttonStates.minButtonState);
    Serial.print(buttonStates.snoozeButtonState);
    Serial.println();
  }
}

void resetDisplay() {
  ledDisplay.drawColon(false);
  ledDisplay.writeDisplay();
}

void drawTimeOnDisplay(int time, bool blinkColon, bool showColon, bool showAlarmActive) {
  bool colon = showColon;

  // Show the current time on the LED display.
  ledDisplay.blinkRate(0);
  ledDisplay.print(time);

  if (!blinkColon) {
    colon = true;
  }

  // Draw right dot indicator to show alarm is on.
  if (showAlarmActive) {
    ledDisplay.writeDigitNum(4, time % 10, true);
  }

  ledDisplay.drawColon(colon);

  // Fill out with zero's if needed
  if (time < 10) {
    ledDisplay.writeDigitNum(0, 0);
    ledDisplay.writeDigitNum(1, 0);
    ledDisplay.writeDigitNum(3, 0);
  }
  else if (time < 100) {
    ledDisplay.writeDigitNum(0, 0);
    ledDisplay.writeDigitNum(1, 0);
  }
  else if (time < 1000) {
    ledDisplay.writeDigitNum(0, 0);
  }

  ledDisplay.writeDisplay();
}

void resetLights() {
  for(byte i= 0; i < RGB_PIXELS;i++){
    rgbPixels.setPixelColor(i, rgbPixels.Color(0, 0, 0));
    rgbPixels.show();
  }
}

void setLight(byte lightNr, RGB rgb, int delayTime) {
  rgbPixels.setPixelColor(lightNr, rgbPixels.Color(rgb.R, rgb.G, rgb.B));
  rgbPixels.show();
  delay(delayTime);
}

void setLights(RGB rgb, int mode) {
  unsigned long currentMillis = millis();
  int ledDelay;
  RGB ledColor = rgb;
  bool blink = true;
  int onOffDelay = 0;

  switch (mode) {
    case RGB_PIXEL_MODE_BLINK_SLOW:
      ledDelay = DELAY_SLOW;
      break;
    case RGB_PIXEL_MODE_BLINK_NORMAL:
      ledDelay = DELAY_NORMAL;
      break;
    case RGB_PIXEL_MODE_BLINK_FAST:
      ledDelay = DELAY_FAST;
      break;
    case RGB_PIXEL_MODE_ON:
      blink = false;
      break;
    case RGB_PIXEL_MODE_BLINK_ON_OFF_SLOW:
      ledDelay = DELAY_SLOW;
      onOffDelay = LED_ON_OFF_DELAY_SLOW;
      break;
    case RGB_PIXEL_MODE_BLINK_ON_OFF_NORMAL:
      ledDelay = DELAY_NORMAL;
      onOffDelay = LED_ON_OFF_DELAY_NORMAL;
      break;
    case RGB_PIXEL_MODE_BLINK_ON_OFF_FAST:
      ledDelay = DELAY_FAST;
      onOffDelay = LED_ON_OFF_DELAY_FAST;
      break;
    case RGB_PIXEL_MODE_OFF:
      ledColor = { 0, 0, 0 };
      blink = false;
      break;
  }

  if (currentMillis - previousLedTime >= ledDelay) {
    previousLedTime = currentMillis;

    if (blink) {
      if (!ledOn) {
        setLight(0, ledColor, onOffDelay);
        setLight(1, ledColor, onOffDelay);
      }
      else {
        setLight(1, { 0, 0, 0 }, onOffDelay);
        setLight(0, { 0, 0, 0 }, onOffDelay);
      }

      ledOn = !ledOn;

      return;
    }

    setLight(0, ledColor, 0);
    setLight(1, ledColor, 0);
  }
}

int handleSettingInput(SettingValue settingValue, ButtonStates buttonStates, int changeBy = 1) {
  int newValue = settingValue.currentValue;

  if (buttonStates.minButtonState == LOW) {
    newValue-= changeBy;
  }
  else if (buttonStates.plusButtonState == LOW) {
    newValue+= changeBy;
  }

  if (newValue < settingValue.minValue) {
    newValue = settingValue.minValue;
  }

  if (newValue > settingValue.maxValue) {
    newValue = settingValue.maxValue;
  }

  return newValue;
}

void drawSetting(int settingValue, DrawSettings drawSettings) {
  if (DRAW_RIGHT_NUMBERS_STYLE) {
    ledDisplay.print(settingValue);

    if (drawSettings.showMenuIndex) {
      if (drawSettings.menuIndex > 9 && drawSettings.menuIndex < 20) {
        ledDisplay.writeDigitNum(0, 1, false);
        ledDisplay.writeDigitNum(1, drawSettings.menuIndex - 10, true);
      }
      else {
        ledDisplay.writeDigitNum(0, drawSettings.menuIndex, true);
      }
    }
  }

  ledDisplay.drawColon(drawSettings.showColon);

  if (drawSettings.blink) {
    ledDisplay.blinkRate(1);
  }
  else {
    ledDisplay.blinkRate(0);
  }

  ledDisplay.writeDisplay();

  delay(drawSettings.delay);
}

void handleMenu(byte type) {
  switch (type) {
    case SOUND_LEVEL_MENU:
      settings.volumeLevel = handleSettingInput({ settings.volumeLevel, MIN_VOLUME, MAX_VOLUME }, buttonStates);
      drawSetting(settings.volumeLevel, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, SOUND_LEVEL_MENU, true });

      mp3Player.volume(settings.volumeLevel);
      break;
    case ALARM_SOUND_MENU:
      settings.activeSound = handleSettingInput({ settings.activeSound, 1, 14 }, buttonStates);
      drawSetting(settings.activeSound, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, ALARM_SOUND_MENU, true });
      break;
    case DISPLAY_BRIGHTNESS_LEVEL_MENU:
      settings.ledBrightness = handleSettingInput({ settings.ledBrightness, LED_BRIGHTNESS_MIN, LED_BRIGHTNESS_MAX }, buttonStates);
      drawSetting(settings.ledBrightness, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, DISPLAY_BRIGHTNESS_LEVEL_MENU, true });

      ledDisplay.setBrightness(settings.ledBrightness);
      break;
    case DISPLAY_COLON_MENU:
      settings.blinkColon = handleSettingInput({ settings.blinkColon, 0, 1 }, buttonStates);
      drawSetting(settings.blinkColon, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, DISPLAY_COLON_MENU, true });
      break;
    case RGB_PIXEL_BRIGTHNESS_LEVEL_MENU:
      settings.rgbPixelBrigthness = handleSettingInput({ settings.rgbPixelBrigthness, RGB_PIXEL_BRIGTHNESS_MIN, RGB_PIXEL_BRIGTHNESS_MAX }, buttonStates);
      drawSetting(settings.rgbPixelBrigthness, { DRAW_RIGHT_NUMBERS_STYLE, false, false, 50, RGB_PIXEL_BRIGTHNESS_LEVEL_MENU, true });

      rgbPixels.setBrightness(settings.rgbPixelBrigthness);
      break;
    case RGB_PIXEL_COLOR_MENU:
      settings.activeLedColor = handleSettingInput({ settings.activeLedColor, 0, (sizeof(LedColors) / sizeof(RGB) -1 ) }, buttonStates);
      drawSetting(settings.activeLedColor, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, RGB_PIXEL_COLOR_MENU, true });

      setLights(LedColors[settings.activeLedColor], settings.ledMode);
      break;
    case RGB_PIXEL_MODE_MENU:
      settings.ledMode = handleSettingInput({ settings.ledMode, LED_MODE_MIN, LED_MODE_MAX }, buttonStates);
      drawSetting(settings.ledMode, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, RGB_PIXEL_MODE_MENU, true });

      setLights(LedColors[settings.activeLedColor], settings.ledMode);
      break;
    case TIME_HOUR_MENU:
      if (newHour == -1) {
        newHour = rtc.now().hour();
      }

      newHour = handleSettingInput({ newHour, 0, 23 }, buttonStates);
      drawSetting(newHour, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, TIME_HOUR_MENU, true });
      break;
    case TIME_MINUTE_MENU:
      if (newMinute == -1) {
        newMinute = rtc.now().minute();
      }

      newMinute = handleSettingInput({ newMinute, 0, 59 }, buttonStates);
      drawSetting(newMinute, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, TIME_MINUTE_MENU, true });
      break;
    case ALARM_TIME_HOUR_MENU:
      settings.alarmHour = handleSettingInput({ settings.alarmHour, 0, 23 }, buttonStates);
      drawSetting(settings.alarmHour, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, ALARM_TIME_HOUR_MENU, true });
      break;
    case ALARM_TIME_MINUTE_MENU:
      settings.alarmMinute = handleSettingInput({ settings.alarmMinute, 0, 59 }, buttonStates);
      drawSetting(settings.alarmMinute, { DRAW_RIGHT_NUMBERS_STYLE, false, true, 100, ALARM_TIME_MINUTE_MENU, true });
      break;
  }
}

void storeSettingInMemory(int address, int value) {
  EEPROM.write(address, value);
}

int retrieveSettingFromMemory(int address, int minValue, int maxValue, int defaultValue) {
  int storedValue = EEPROM.read(address);

  if (storedValue >= minValue && storedValue <= maxValue) {
     return storedValue;
  }

  return defaultValue;
}

void enableAlarm() {
  playAlarm = true;
  mp3Player.loop(settings.activeSound);
}

void muteAlarm() {
  alarmActive = false;
  playAlarm = false;
  digitalWrite(SNOOZE_LED, LOW);
  mp3Player.stop();
}

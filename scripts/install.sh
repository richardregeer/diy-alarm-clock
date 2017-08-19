# Pre cleanup
rm -rf *.tar.gz*
rm -rf libraries

# Create folder and download libraries
mkdir libraries
wget -O neopixel.tar.gz https://github.com/adafruit/Adafruit_NeoPixel/archive/v1.1.2.tar.gz
tar -x -f neopixel.tar.gz -C libraries/

wget -O rtc.tar.gz https://github.com/adafruit/RTClib/archive/1.2.0.tar.gz
tar -x -f rtc.tar.gz -C libraries/

wget -O led-backpack.tar.gz https://github.com/adafruit/Adafruit_LED_Backpack/archive/1.1.6.tar.gz
tar -x -f led-backpack.tar.gz -C libraries/

wget -O gfx.tar.gz https://github.com/adafruit/Adafruit-GFX-Library/archive/1.2.2.tar.gz
tar -x -f gfx.tar.gz -C libraries/

wget -O dfplayer.tar.gz https://github.com/DFRobot/DFRobotDFPlayerMini/archive/1.0.1.tar.gz
tar -x -f dfplayer.tar.gz -C libraries/

# Post cleanup
rm -rf *.tar.gz*

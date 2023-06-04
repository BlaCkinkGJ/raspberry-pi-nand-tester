# NAND Tester using Raspberry Pi 3
 K9F1G08U0E NAND Test in Raspberry Pi (https://www.waveshare.com/wiki/NandFlash_Board_(A)).
 
 For convenience development, I am using the [wiringPi](https://github.com/WiringPi/WiringPi) library.
 
 Pin Settings
 
 - WiringPi GPIO 0 ~ 8 for I/O Pins (0(NAND) to 0(Pi), 8 to 8)
 - WiringPi GPIO 21 ~ 27 for Command Pins(CE, RE, ALE ...)

This program executes the memory sanitizer. Therefore, you must install `libasan6`.

```bash
sudo apt install -y libasan6
```

## Schematic

Two WiringPi modules are derived from one RaspberyPi 3. Note that Wiring GPIO numbers can be different from the BCM number.

![KakaoTalk_20230605_024338060](https://github.com/BlaCkinkGJ/raspberry-pi-nand-tester/assets/16631264/8f43f659-18c5-40d6-bf76-5c305fb3f471)


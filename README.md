# STM32duino

This are some project I build using Arduino IDE and STM32duino software.
I use the STM32F746-Disco board and, following some good trik from Internet, I 'm be able to run program using
the LCD, the Touchscreen and the external SDRAM.

The CMSIS-DSP is also working well.

This are somw link that help me a lot to make this working:

  Follow this link for access externa SDRAM:
  https://www.stm32duino.com/viewtopic.php?t=759
  
 Follow this in case of "fork/exec" error:
 ## Customized platform.local.txt to compile for STM32 targets under Windows
 ## This recipe works around the Windows limit of 32k characters in a cmd line by calling .o files from a text file during linking
 ## Install: put file under your mbed sub-directory - ie: C:\Users\MYUSER\AppData\Local\Arduino15\packages\arduino\hardware\stm32\1.9.0
 ## You can also find the local arduino folder in the IDE under File -> Preferences -> see preferences.txt location in the bottom part of the window
  
 * https://forum.edgeimpulse.com/t/filename-or-extension-too-long-stm32f401re/874

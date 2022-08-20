/*
 * 
 * Follow this link for access externa SDRAM:
 * https://www.stm32duino.com/viewtopic.php?t=759
 * 
 * Follow this in case of "fork/exec" error:
 * ## Customized platform.local.txt to compile for STM32 targets under Windows
 * ## This recipe works around the Windows limit of 32k characters in a cmd line by calling .o files from a text file during linking
 * ## Install: put file under your mbed sub-directory - ie: C:\Users\MYUSER\AppData\Local\Arduino15\packages\arduino\hardware\stm32\1.9.0
 * ## You can also find the local arduino folder in the IDE under File -> Preferences -> see preferences.txt location in the bottom part of the window
 * 
 * https://forum.edgeimpulse.com/t/filename-or-extension-too-long-stm32f401re/874
 * 
 * 
 * To convert and resize movie files I use this command:
 * ffmpeg.exe -i fname.avi -vf "fps=15,scale=-1:270:flags=lanczos" -pix_fmt yuvj420p -q:v 9 fname.mjpeg
 * 
 * the scale=-1:270 rescale mantaining the propotion of 270 pixel height
 * 
 * 
 * To modify heap and/or stack memory size, open:
 * C:\Users\<USER>\AppData\Local\Arduino15\packages\STM32\hardware\stm32\1.9.0\variants\DISCO_F746NG\ldscript.ld
 * 
 */

#define FIRMWARE_VERSION "0.01"

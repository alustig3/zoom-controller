![hello](media/TITLE2.BMP)
![hello](media/both3.BMP)
# What is this?
There has been a huge spike in Zoom usage this year. Zoom fatigue has become a thing. I created this device to make interacting with the Zoom software faster and more pleasant.

The Zoom Controller connects with your computer over Bluetooth and emulates a keyboard key presses. The buttons are setup to emulate the default keyboard shortcuts in Zoom.  

# How do I use it?
## Turn On
Press and hold the volume knob to turn on. The volume knob will "breathe" red while it is trying to connect. Once connected, the knob will turn green. 

The controller will automatically turn off after 20 seconds attempting and failing to connect.

## Turn Off
Press and hold the LEAVE button. The volume knob's green light will turn off when the controller is powered off.

## Adjust Computer Volume
Rotate the VOLUME knob clockwise to increase volume, counterclockwise to decrease volume.

## Button Functions
|              | PCB Label | Emulate Mac Keypress | Emulate Windows Keypress | Zoom Function                  |
|--------------|-----------|----------------------|--------------------------|--------------------------------|
| **Button 1** | Mic       | shift + command + a  |                          | mute/unmute microphone         |
| **Button 2** | Camera    | shift + command + v  |                          | start/stop video               |
| **Button 3** | Share     | shift + command + s  |                          | start/stop screen sharing      |
| **Button 4** | View      | shift + command + w  |                          | switch to speaker/gallery view |
| **Button 5** | Leave     | command + w          |                          | leave meeting                  |

There is an idle clock always counting. Every time you press a button or rotate the knob, a idle clock is reset to 0. If the idle clock reaches 15 minutes, the controller will automatically turn itself off to save battery.

## Getting Fancy
### Mac Notifications
I downloaded [Hammerspoon](https://www.hammerspoon.org/)
Click the Hammerspooon toolbar icon and select "Open Config".
Add the following to you init.lua file:
```lua
hs.hotkey.bind({"cmd", "alt", "ctrl"}, "F1", function()
  hs.alert.show("Zoom Mode")
end)

hs.hotkey.bind({"cmd", "alt", "ctrl"}, "F2", function()
  hs.alert.show("YouTube Mode")
end)

hs.hotkey.bind({"cmd","alt","ctrl"}, "f3", function()
  hs.notify.new({title="Zoom Controller", informativeText="Controller has been idle for 5 minutes\n Press key to keep awake"}):send()
end)
```
### Windows Notifications
AutoHotkey


### YouTube Controller
I've found this interface to be super useful for watching videos. I installed the [Video Speed Controller](https://chrome.google.com/webstore/detail/video-speed-controller/nffaoalbilbmmfgbnbgppjihopabppdk) extension. 


# How do I Build one?
## Bill of Materials
| Qty | Reference          | Description       | Value/MPN                                                                                                    | 
|-----|--------------------|-------------------|--------------------------------------------------------------------------------------------------------------|
| 1   | BT1                | Battery Holder    | [2462](https://www.digikey.com/product-detail/en/keystone-electronics/2466/36-2466-ND/303815)                | 
| 1   | C1                 | 0805 Capacitor    | [10nF](~)                                                                                                    | 
| 1   | D1                 | Schottky Diode    | [BAT54C](http://www.diodes.com/_files/datasheets/ds11005.pdf)                                                | 
| 2   | H3, H4             | Threaded Standoff | [4207](https://www.adafruit.com/product/4207)                                                                | 
| 1   | Q1                 | P-Channel MOSFET  | [FDN340P](https://www.digikey.com/products/en?keywords=FDN340PCT-ND)                                         | 
| 1   | Q2                 | N-Channel MOSFET  | [BSS123](https://www.digikey.com/products/en?keywords=BSS123LT1GOSCT-ND)                                     | 
| 1   | R1                 | 0805 Resistor     | [100KΩ](~)                                                                                                   | 
| 1   | R2                 | 0805 Resistor     | [10KΩ](~)                                                                                                    | 
| 1   | R3                 | 0805 Resistor     | [330Ω](~)                                                                                                    | 
| 1   | R4                 | 0805 Resistor     | [1MΩ](~)                                                                                                     | 
| 5   | S1, S2, S3, S4, S5 | Tactile Button    | [DD-15326](https://www.sparkfun.com/products/15326)                                                          | 
| 1   | SW1                | Rotary Encoder    | [COM-15140](https://www.sparkfun.com/products/15140)                                                         | 
| 1   | U1                 | Microcontroller   | [ESP32-WROOM-32E](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-WROOM-32E-4MB/11613125) | 


# How do I buy one?
If people are interested, I may consider buying some parts in bulk and selling 
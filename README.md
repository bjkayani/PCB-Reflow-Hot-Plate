# PCB Reflow Hot Plate

The PCB Reflow Hot Plate is a temperature controlled 120 x 70 mm hot plate designed to reflow solder SMD circuit boards. This project was started due to a gap in the commercial solutions for a mid sized form factor hot plate. 

This project is a work in progress and various elements of it are continually improving and changing. Refer to the build guide for more information including a compatibility table. 

<br>

<img src="https://i.ibb.co/6Y3WBQz/IMG-2905.jpg" width="80%"></a>

<br>

## Features

- 120x70mm (4.7x2.7in) aluminum hot plate
- 300 Watt PTC heating element
- ESP32-C3 based control board
- Tuned PID controller with configurable parameters
- Reflow mode with customizable reflow profiles
- Constant temeprature (heat) mode
- Upto 260 C temperature allows lead free reflow
- Easy to use GUI using 0.96" OLED display and 3 buttons
- Serial debugging over USB to monitor temperature
- Single power cable required for operation

<br>

## Design

### The main design considerations were:
- Atleast 70x70mm reflow area (the Miniware is 30x30mm)
- Slim design to allow tweezer manipulation with hands resting on the desk
- Commercially viable solution with a long operating life
- Easy to operature and minimal setup process
- Safe operation with multiple layers of safety

With these considerations in mind, this is the design I came up with. It uses an off the shelf heating plate with an embedded 300W heating element. I designed my own control board that includes everything needed to control the heating plate and can run entirely from mains input. 

I went with a small 0.96" OLED display and three buttons as that seems adequate to control the UI. The UI is simple and intuitive yet provides all the functionality needed to operate and customize the hot plate. 

Its all packaged in a slim, rectangular 3D printed case that exposes the hot plate provide a window for the display and integrated buttons. The hotplate has insulation placed underneath it to prevent heat loss from the bottom. The case needs to be printed in a relatively temperature resistant material to avoid warping at high temperature. 

<img src="https://i.ibb.co/jz0Cfwf/IMG-2908.jpg" width="50%"></a>

<br>

## PCB

The PCB is designed in KiCAD. Its a 2-layer design and it contains everything needed to run the hot plate with a single mains input. The USB-C port provides power to the DC section and allows for programming the ESP32-C3 microcontroller. The OLED display is on a separate daughter board and mounted using M2 standoffs. A standard 0.96" OLED display module should work although there is some variation in the models avaiable. More information regarding that coming soon. 

### Version 1.1:

- [BOM](PCB/pcb-reflow-hot-plate/bom/pcb-reflow-hot-plate_1.1.html)
- [Gerber](PCB/pcb-reflow-hot-plate/output/pcb-reflow-hot-plate_Rev1.1.zip)

### Revision History

- Rev 1.0
    - Initial design
- Rev 1.1
    - AC-DC converter changed to HLK-5M05
    - SSR changed to popular OMRON compatible footprint
    - OLED display module moved to center align with buttons
    - SSR supply changed to +5V
    - AC in connector mounting peg holes increased in size
    - Minor silk screen updates

<img src="https://i.ibb.co/8XnHh54/pcb-reflow-hot-plate.png" width="40%"></a>
<img src="https://i.ibb.co/NC9FkzB/Screenshot-2023-04-15-225138.png" width="40%"></a>

<br>

## Code

The code is written in Arduino IDE due to its ease of use and simlicity over PlatformIO and native ESP SDK. The code is split up into multiple files with each file containing functions relating to specific funcionality. 

The code is continually under development and I am adding new features regularly. I have also commented the code so that its easy to understand. Feel free to look through the code, file an issue if you encounter one or request features to be added. 

I have a lot of features I want to add that are on the road map but I first wanted to get the basic funtionality working and tested. These are listed at the start of the [main program file](Code/pcb-reflow-hot-plate/pcb-reflow-hot-plate.ino). 

<br>

## CAD

The housing is designed in SolidWorks. Its designed as a two part enclosure with a tab and screws to hold the two together. The heating plate comes with silicon feet that fit into the base. The base is raised to be slighly protruding from the top surface and insulation is put underneath to avoid heat dissipation from the bottom. 

The latest STL and design files can be found in the CAD folder. The assembly is missing the PCB due to import issues from KiCAD into SolidWorks.

<img src="https://i.ibb.co/VNH4gkz/Screenshot-2023-04-16-001648.png" width="40%"></a>
<img src="https://i.ibb.co/88zyrQJ/Screenshot-2023-04-16-004012.png" width="40%"></a>

<br>

## Build Guide

### Build guide is coming soon ...

### Compatibility Table

| PCB (HW) | OLED Module | Code (FW) | CAD |
| :---: | :---: | :---: | :---: |
| Rev 1.0 | 25.2x26mm <br>[ AliExpress Link](https://www.aliexpress.us/item/2251832644208699.html?spm=a2g0o.order_list.order_list_main.11.21ef1802tOmkk3&gatewayAdapt=glo2usa4itemAdapt&_randl_shipto=US#nav-specification) | Rev 1.0 | Housing Top v3 <br> Housing Base v7 |

<br>


## Future Ideas
- Add mounting holes and pogo pin body connectors for accessories such as cooling fan, external temperature probe, fume extractor
- Make display separate from main PCB and shrink the main PCB
- Move the buttons to the bottom of the display
- Increase the size of the display to ~1.5in
- Switch to DC heater with thinner hot plate and USB-C PD (100 Watts)
- Switch to a square heating element
- Add integrated active cooling that blows air parallel to the hot plate
- Add multiple sensing options (thermocouple, thermistor etc.)
- Add AC protection fuse
# PCB Reflow Hot Plate

The PCB Reflow Hot Plate is a temperature controlled 12 x 7 cm hot plate designed to reflow solder SMD circuit boards. This project was started out of a gap in the commercial solutions for a mid sized form factor hot plate. 


<img src="https://i.ibb.co/gvjpGFz/IMG-2859.jpg" width="500px"></a>


## Features

- 120x70mm (4.7x2.7in) aluminum hot plate
- 300 Watt PTC heating element
- ESP32-C3 based control board
- Tuned PID controller with configurable parameters
- Reflow mode with customizable reflow profiles
- Constant temeprature (heat) mode
- Upto 260 C temperature allows lead free reflow
- Easy to use GUI using OLED display and 3 buttons
- Serial debugging over USB to monitor temperature
- Single power cable required for operation

## Design

## PCB

## Programming

## Code

## CAD

## Futher Tasks

## New Ideas
- Add mounting holes and pogo pin body connectors for accessories such as cooling fan, external temperature probe, fume extractor
- Make display separate from main PCB and shrink the main PCB
- Move the buttons to the bottom of the display
- Increase the size of the display to ~1.5in
- Switch to DC heater with thinner hot plate and USB-C PD (100 Watts)
- Switch to a square heating element
- Add integrated active cooling that blows air parallel to the hot plate
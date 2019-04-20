# DroneSim
A physics simulation for a quad-copter drone

## Requirements
- Unity Version 2018.3.0
 
## Adjusting Simulation Values
To edit any relevant drone properties, click `Drone Parent` under the `Hierarchy` tab. This will display all of the drone's adjustable data in the `Inspector` tab. The following values can be adjusted: 

#### Under Rigidbody
- Mass: The mass of the drone in kg

#### Under Drone Movement Script
- Air Density: The density of air in kg/m<sup>3</sup>
- Lift Coefficient: A dimensionless constant that is normally obtained experimentally
- Propeller Area: The surface area of a propeller in m<sup>2</sup>
- Propeller Diameter: The diameter of the propeller (tip-to-tip) in inches
- RPM Max: The highest allowable RPM for the propeller motors in revolutions per minute
- RPM Min: The lowest allowable RPM for the propeller motors in revolutions per minute
- Tilt Angle Forward: The forward/backward angle tilt of the drone in degrees 
- Tilt Angle Right: The left/right angle tilt of the drone in degrees

## How to Use Drone
- `W` to move forward
- `S` to move backward
- `A` to move left
- `D` ro move right
- `I` to move upward
- `K` to move downward
- `J` to rotate CCW (with rotation enabled)
- `L` to rotate CW (with rotation enabled)

## Collaborators
- Joshua Riley
- Adriel Bustamente
- Jonathan Woolf
- Colton Vosburg

### BOMBOCLATTTTT
to re-build:
``` powershell
cmake --preset Debug; if ($?) { cmake --build --preset Debug --clean-first }
```

Workpath
addressing feedback:
* Figure out what the structure of PrimitiveIO is suppossed to do, come up with a plan to move these functions to the common library
* Figure out pin defs, and 
* implement changes to calibration according to Rowans comments. 
    * Probably poll time since last update or something instead of interrupts (overkill, calibration can be costly time-wise)
* Figure out if you can define pins in multiple places, that way you have a .h file with all the pins on the platform similar to mini-rover, and then cubemx pindefs and stuff
* After addressing these changes, stage a commit and push saying ready for hardware-in-loop

* after double checking that it is functional, prepare for CAN integration with ROS 

Workpath for UART library
* figure out git worktree
* figure out COBS + CRC... I think this is good enough for something basic
* integration into common_library

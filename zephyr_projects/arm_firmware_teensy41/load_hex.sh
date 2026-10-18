echo LOADING HEX
sudo teensy_loader_cli -w -v --mcu TEENSY41 build/zephyr/zephyr.hex 

# if its failing a lot on the first try, this was our bandaid:
#sleep 1.0
#sudo teensy_loader_cli -w -v --mcu TEENSY41 build/zephyr/zephyr.hex 

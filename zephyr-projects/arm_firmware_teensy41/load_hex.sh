    source ~/zephyrproject/zephyr/zephyr-env.sh
    source ~/zephyrproject/.venv/bin/activate

if [ $1 -gt 5 ]; then
    mv teensy.overlay app.overlay
    west build -p auto -b teensy41
    mv app.overlay teensy.overlay

    sudo teensy_loader_cli -w -v --mcu TEENSY41 build/zephyr/zephyr.hex
    sleep 1.0
    sudo teensy_loader_cli -w -v --mcu TEENSY41 build/zephyr/zephyr.hex 
else
    mv esp32.overlay app.overlay
    west build -p auto -b esp32 

    # echo LOADING HEXs
    west flash
    mv app.overlay esp32.overlay
    # west espressif monitor
    screen /dev/ttyUSB0 115200
fi

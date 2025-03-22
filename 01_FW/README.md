# KeyBird Firmware


### Debug Tips

#### Unable to flash
```
[error] [ Client] - Encountered error -102: Command read_device_info executed for 125 milliseconds with result -102
[error] [ Worker] - An unknown error.
[error] [ Client] - Encountered error -102: Command read_memory_descriptors executed for 24 milliseconds with result -102
Failed to read device memories.
[error] [ Worker] - An unknown error.
ERROR: JLinkARM DLL reported an error. Try again. If error condition
ERROR: persists, run the same command again with argument --log, contact Nordic
ERROR: Semiconductor and provide the generated log.log file to them.
NOTE: For additional output, try running again with logging enabled (--log).
NOTE: Any generated log error messages will be displayed.
FATAL ERROR: command exited with status 33: nrfjprog --program /home/timmy/proj/hid_keyboard/01_FW/build_dbg_left/zephyr/zephyr.hex --sectoranduicrerase --verify -f NRF52 --snr 1050051174
```
The following error seems to occur occasionally. I'm not sure what causes it. 
The following solutions are recommended, but they've never worked for me. 
`nrfjprog --recover -f nrf52 --log` 
`nrfjprog --eraseall -f nrf52 --log` 

What worked for me was connecting to JLink directly. 
It seems that you may need to discuss the FTDI cable when doing this. 

```
JLinkExe
>> connect
```
Select 
- Device: nRF52840_xxAA
- Interface: SWD
- Speed: 4000

```
>> w4 4001e504 2 
>> w4 4001e400 1 
>> exit
```

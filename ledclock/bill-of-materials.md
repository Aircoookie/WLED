# Bill of Materials

This page lists all parts and materials required to build the clock.

## 3D-printing

To be able to print the base of the enclosure, a printer with a large build volume is required, since its ~40 cm wide. All other parts can be printed on a regular sized printers.

You need at least two kinds of material, an opaque and a translucent one. See the tables on [this page](models/3d-print/readme.md) for the details and required amounts.

## Electronics

First of all, you need the PCBs. Making them yourself could be quite a challenge since tha main board is a double sided design and the other two boards have a non-square outline which can be best CNC-milled or hand made with a router. Therefore I would recommend ordering them form a prototyping PCB service. I had them made at JLC PCB and the quality they arrived in is very convincing. Check out the [designs](sch-pcb/readme.md) and [gerber files](gerber/readme.md).

The following table shows the required components. The datasheets show the exact pieces I used in my design and realization.

| Part/Material | Quantity | Notes |
|---------------|---------:|-------|
| Power supply | 1 | 5V DC regulated, with at least 3000 mA output current rating.<br>Smaller one could be used with firmware modification. Inner contact of the plug is the positive. |
| [Power connector](datasheets/pc-gk2.1.pdf) | 1 | Obviously has to match the PSU plug. |
| [WS2812b LED chip](datasheets/ws2812b.pdf) | 58 | |
| [Piezo buzzer](datasheets/sfn-12055pa6.5_en_10040569.pdf) | 1 | A passive buzzer is necessary. |
| [Electrolytic capacitor 470u](datasheets/sc-caps.pdf) | 1 | |
| [Trimmer potentiometer 47k](datasheets/ca6v-smd47k_en_10032410.pdf) | 1 | With linear scale. |
| [BSS138 N-mosfet](datasheets/bss138_en_10021022.pdf) | 1 | |
| [MMBT2222A transistor](datasheets/mmbt2222a_en_10039962.pdf) | 2 | |
| [AMS1117 voltage 3.3V voltage regulator](datasheets/ams1117.pdf) | 1 | |
| [Micro USB connector](datasheets/10118193-0001lf_en_10038704.pdf) | 1 | |
| Ceramic capacitor 10u, 10V, 0805 | 3 | |
| Ceramic capacitor 1u, 16V, 0603 | 2 | |
| Ceramic capacitor 100n, 50V, 0603 | 58 | |
| Resistor 100R, 5%, 0603 | 1 | |
| Resistor 1k, 5%, 0603 | 1 | |
| Resistor 10k, 5%, 0603 | 5 | |
| Resistor 22.1k, 1%, 0603 | 1 | |
| Resistor 47.5k, 1%, 0603 | 1 | |
| [TEPT5700 ambient light sensor](datasheets/tept5700.pdf) | 1 | See the [application notes](datasheets/appnote-tept5700.pdf). |
| [ESP32-WROOM-32E 4MB](datasheets/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf) | 1 | See the [pinout](datasheets/esp32-pinout-chip-esp-wroom-32.png). |
| [CP2102N USB->UART interface](datasheets/cp2102n-datasheet.pdf) | 1 | |
| [4-pin wire to board connector male 90deg. with 2mm raster](datasheets/nxw-04k_en_10034352.gif) | 5 | Either SMD or THT, depeding the PCB you choose. |
| [4-pin wire to board connector female with 2mm raster](datasheets/nxg-04_en_10031380.pdf) | 5 | |
| [4-pin wire to board connector pin](datasheets/nxg-t_en_10030915.pdf) | 20 | |
| 0.22 mm<sup>2</sup> stranded wire | 58 | 4 distinct colors; quantity is in cm and is for each color separately. |
| [M3 threaded inserts](https://www.cnckitchen.com/shop#!/Gewindeeinsatz-threaded-insert-M3-Standard-100-Stk-pcs/p/431146823/category=0) | 6 | |
| M3 6mm screws | 6 | |
| [Self-adhesive plastic legs](datasheets/elastikpuffer.pdf) | 8 | |
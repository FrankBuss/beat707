EESchema Schematic File Version 4
LIBS:beat707-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 4200 2900 1550 1000
U 5C65184E
F0 "TM1638 Module" 50
F1 "tm1638-module.sch" 50
$EndSheet
$Sheet
S 3100 4600 1550 1000
U 5C6557AC
F0 "sheet5C6557AC" 50
F1 "tm1638-module.sch" 50
$EndSheet
$Sheet
S 5250 4600 1550 1000
U 5C6559D3
F0 "sheet5C6559D3" 50
F1 "tm1638-module.sch" 50
$EndSheet
$Comp
L MCU_Microchip_ATmega:ATmega328-PU U1
U 1 1 5C655D4D
P 2000 4350
F 0 "U1" H 1359 4396 50  0000 R CNN
F 1 "ATmega328-PU" H 1359 4305 50  0000 R CNN
F 2 "Package_DIP:DIP-28_W7.62mm" H 2000 4350 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega328_P%20AVR%20MCU%20with%20picoPower%20Technology%20Data%20Sheet%2040001984A.pdf" H 2000 4350 50  0001 C CNN
	1    2000 4350
	1    0    0    -1  
$EndComp
$Comp
L Connector:USB_B J1
U 1 1 5C655E8B
P 7000 2750
F 0 "J1" H 7055 3217 50  0000 C CNN
F 1 "USB_B" H 7055 3126 50  0000 C CNN
F 2 "Connector_USB:USB_B_OST_USB-B1HSxx_Horizontal" H 7150 2700 50  0001 C CNN
F 3 " ~" H 7150 2700 50  0001 C CNN
	1    7000 2750
	1    0    0    -1  
$EndComp
$Comp
L Connector:AudioJack3_Ground J3
U 1 1 5C6560D3
P 8000 5150
F 0 "J3" H 8004 5492 50  0000 C CNN
F 1 "AudioJack3_Ground" H 8004 5401 50  0000 C CNN
F 2 "Connector_Audio:Jack_3.5mm_PJ311_Horizontal" H 8000 5150 50  0001 C CNN
F 3 "~" H 8000 5150 50  0001 C CNN
	1    8000 5150
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5_180degree J2
U 1 1 5C65661A
P 8550 2800
F 0 "J2" H 8550 2526 50  0000 C CNN
F 1 "DIN-5_180degree" H 8550 2435 50  0000 C CNN
F 2 "beat707:DIN-Socket" H 8550 2800 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/18/40_c091_abd_e-75918.pdf" H 8550 2800 50  0001 C CNN
	1    8550 2800
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5_180degree J4
U 1 1 5C656650
P 8550 3750
F 0 "J4" H 8550 3476 50  0000 C CNN
F 1 "DIN-5_180degree" H 8550 3385 50  0000 C CNN
F 2 "beat707:DIN-Socket" H 8550 3750 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/18/40_c091_abd_e-75918.pdf" H 8550 3750 50  0001 C CNN
	1    8550 3750
	1    0    0    -1  
$EndComp
$Comp
L switch:SW_Push SW?
U 1 1 5C74CDB9
P 3350 2650
AR Path="/5C65184E/5C74CDB9" Ref="SW?"  Part="1" 
AR Path="/5C6557AC/5C74CDB9" Ref="SW?"  Part="1" 
AR Path="/5C6559D3/5C74CDB9" Ref="SW?"  Part="1" 
AR Path="/5C74CDB9" Ref="SW25"  Part="1" 
F 0 "SW25" H 3350 2935 50  0000 C CNN
F 1 "SW_Push" H 3350 2844 50  0000 C CNN
F 2 "beat707:K2-1103SP-A4SW-04" H 3350 2850 50  0001 C CNN
F 3 "" H 3350 2850 50  0001 C CNN
	1    3350 2650
	1    0    0    -1  
$EndComp
$Comp
L switch:SW_Push SW?
U 1 1 5C74CDF8
P 3350 3200
AR Path="/5C65184E/5C74CDF8" Ref="SW?"  Part="1" 
AR Path="/5C6557AC/5C74CDF8" Ref="SW?"  Part="1" 
AR Path="/5C6559D3/5C74CDF8" Ref="SW?"  Part="1" 
AR Path="/5C74CDF8" Ref="SW26"  Part="1" 
F 0 "SW26" H 3350 3485 50  0000 C CNN
F 1 "SW_Push" H 3350 3394 50  0000 C CNN
F 2 "beat707:K2-1103SP-A4SW-04" H 3350 3400 50  0001 C CNN
F 3 "" H 3350 3400 50  0001 C CNN
	1    3350 3200
	1    0    0    -1  
$EndComp
$Comp
L switch:SW_Push SW?
U 1 1 5C74CE2B
P 3400 3750
AR Path="/5C65184E/5C74CE2B" Ref="SW?"  Part="1" 
AR Path="/5C6557AC/5C74CE2B" Ref="SW?"  Part="1" 
AR Path="/5C6559D3/5C74CE2B" Ref="SW?"  Part="1" 
AR Path="/5C74CE2B" Ref="SW27"  Part="1" 
F 0 "SW27" H 3400 4035 50  0000 C CNN
F 1 "SW_Push" H 3400 3944 50  0000 C CNN
F 2 "beat707:K2-1103SP-A4SW-04" H 3400 3950 50  0001 C CNN
F 3 "" H 3400 3950 50  0001 C CNN
	1    3400 3750
	1    0    0    -1  
$EndComp
$Comp
L Device:Rotary_Encoder SW28
U 1 1 5C74D2CA
P 7000 3750
F 0 "SW28" H 7229 3796 50  0000 L CNN
F 1 "Rotary_Encoder" H 7229 3705 50  0000 L CNN
F 2 "Rotary_Encoder:RotaryEncoder_Alps_EC12E_Vertical_H20mm" H 6850 3910 50  0001 C CNN
F 3 "~" H 7000 4010 50  0001 C CNN
	1    7000 3750
	1    0    0    -1  
$EndComp
$EndSCHEMATC

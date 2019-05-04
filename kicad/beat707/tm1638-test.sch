EESchema Schematic File Version 4
LIBS:tm1638-test-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 2
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
F2 "STB" I R 5750 3250 50 
F3 "CLK" I R 5750 3350 50 
F4 "DIO" I R 5750 3450 50 
F5 "GND" I R 5750 3550 50 
F6 "VCC" I R 5750 3150 50 
$EndSheet
$Comp
L Connector:Conn_01x05_Male J1
U 1 1 5C8D46A1
P 6350 3350
F 0 "J1" H 6350 3650 50  0000 C CNN
F 1 "Conn_01x05_Male" H 6350 3050 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 6350 3350 50  0001 C CNN
F 3 "~" H 6350 3350 50  0001 C CNN
	1    6350 3350
	-1   0    0    1   
$EndComp
Wire Wire Line
	5750 3150 6150 3150
Wire Wire Line
	6150 3250 5750 3250
Wire Wire Line
	5750 3350 6150 3350
Wire Wire Line
	6150 3450 5750 3450
Wire Wire Line
	5750 3550 6150 3550
$EndSCHEMATC

EESchema Schematic File Version 4
LIBS:beat707-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 8
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Transistor_FET:2N7000 Q?
U 1 1 5CCE7C42
P 5200 3350
AR Path="/5CCE7C42" Ref="Q?"  Part="1" 
AR Path="/5CCE7B30/5CCE7C42" Ref="Q1"  Part="1" 
AR Path="/5CD0BEF5/5CCE7C42" Ref="Q2"  Part="1" 
AR Path="/5CD1492F/5CCE7C42" Ref="Q3"  Part="1" 
AR Path="/5CD1D369/5CCE7C42" Ref="Q4"  Part="1" 
F 0 "Q4" H 5400 3425 50  0000 L CNN
F 1 "2N7000" H 5400 3350 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 5400 3275 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7000.pdf" H 5200 3350 50  0001 L CNN
	1    5200 3350
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 5CCE7C49
P 4800 3200
AR Path="/5CCE7C49" Ref="R?"  Part="1" 
AR Path="/5CCE7B30/5CCE7C49" Ref="R119"  Part="1" 
AR Path="/5CD0BEF5/5CCE7C49" Ref="R121"  Part="1" 
AR Path="/5CD1492F/5CCE7C49" Ref="R123"  Part="1" 
AR Path="/5CD1D369/5CCE7C49" Ref="R125"  Part="1" 
F 0 "R125" V 4880 3200 50  0000 C CNN
F 1 "10k" V 4800 3200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 4730 3200 50  0001 C CNN
F 3 "~" H 4800 3200 50  0001 C CNN
	1    4800 3200
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 5CCE7C50
P 4800 2900
AR Path="/5CCE7C50" Ref="#PWR?"  Part="1" 
AR Path="/5CCE7B30/5CCE7C50" Ref="#PWR059"  Part="1" 
AR Path="/5CD0BEF5/5CCE7C50" Ref="#PWR061"  Part="1" 
AR Path="/5CD1492F/5CCE7C50" Ref="#PWR063"  Part="1" 
AR Path="/5CD1D369/5CCE7C50" Ref="#PWR065"  Part="1" 
F 0 "#PWR065" H 4800 2750 50  0001 C CNN
F 1 "+3.3V" H 4800 3040 50  0000 C CNN
F 2 "" H 4800 2900 50  0001 C CNN
F 3 "" H 4800 2900 50  0001 C CNN
	1    4800 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 3350 4800 3450
Wire Wire Line
	4800 3450 5000 3450
Wire Wire Line
	4800 3050 4800 2950
Wire Wire Line
	4800 2950 5200 2950
Wire Wire Line
	5200 2950 5200 3150
Connection ~ 4800 2950
Wire Wire Line
	4800 2950 4800 2900
Wire Wire Line
	4800 3450 4600 3450
Connection ~ 4800 3450
$Comp
L power:+5V #PWR?
U 1 1 5CCE7C5F
P 5550 2900
AR Path="/5CCE7C5F" Ref="#PWR?"  Part="1" 
AR Path="/5CCE7B30/5CCE7C5F" Ref="#PWR060"  Part="1" 
AR Path="/5CD0BEF5/5CCE7C5F" Ref="#PWR062"  Part="1" 
AR Path="/5CD1492F/5CCE7C5F" Ref="#PWR064"  Part="1" 
AR Path="/5CD1D369/5CCE7C5F" Ref="#PWR066"  Part="1" 
F 0 "#PWR066" H 5550 2750 50  0001 C CNN
F 1 "+5V" H 5550 3040 50  0000 C CNN
F 2 "" H 5550 2900 50  0001 C CNN
F 3 "" H 5550 2900 50  0001 C CNN
	1    5550 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5CCE7C65
P 5550 3200
AR Path="/5CCE7C65" Ref="R?"  Part="1" 
AR Path="/5CCE7B30/5CCE7C65" Ref="R120"  Part="1" 
AR Path="/5CD0BEF5/5CCE7C65" Ref="R122"  Part="1" 
AR Path="/5CD1492F/5CCE7C65" Ref="R124"  Part="1" 
AR Path="/5CD1D369/5CCE7C65" Ref="R126"  Part="1" 
F 0 "R126" V 5630 3200 50  0000 C CNN
F 1 "10k" V 5550 3200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal" V 5480 3200 50  0001 C CNN
F 3 "~" H 5550 3200 50  0001 C CNN
	1    5550 3200
	-1   0    0    1   
$EndComp
Wire Wire Line
	5400 3450 5550 3450
Wire Wire Line
	5550 3450 5550 3350
Wire Wire Line
	5550 2900 5550 3050
Wire Wire Line
	5550 3450 5700 3450
Connection ~ 5550 3450
Text HLabel 4800 2950 0    50   Input ~ 0
+3.3V
Text HLabel 5550 2950 0    50   Input ~ 0
+5V
Text HLabel 4600 3450 0    50   Input ~ 0
low-side
Text HLabel 5700 3450 2    50   Input ~ 0
high-side
$EndSCHEMATC

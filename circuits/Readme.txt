This is the Readme file for the V-USB example circuits directory.


CIRCUITS IN THIS DIRECTORY
==========================
Since USB requires 3.3 V levels on D+ and D- but delivers a power supply of
ca. 5 V, some kind of level conversion must be performed. There are several
ways to implement this level conversion, see the example circuits below.

with-vreg.png and with-vreg.sch (EAGLE schematics):
  This circuit uses a low drop voltage regulator to reduce the USB supply to
  3.3 V. You MUST use a low drop regulator because standard regulators such
  as the LM317 require at least ca. 2 V drop. The advantage of this approach
  is that it comes closest to the voltage levels required by the USB
  specification and that the circuit is powered from a regulated supply. If
  no USB cable is used (connector directly soldered on PCB), you can even
  omit the 68 Ohm series resistors. The disadvantage is that you may want to
  use other chips in your design which require 5 V. Please check that the AVR
  used in your design allows the chosen clock rate at 3.3 V.

with-zener.png and with-zener.sch (EAGLE schematics):
  This circuit enforces lower voltage levels on D+ and D- with zener diodes.
  The zener diodes MUST be low power / low current types to ensure that the
  1k5 pull-up resistor on D- generates a voltage of well above 2.5 V (but
  below 3.6 V). The advantage of this circuit is its simplicity and that the
  circuit can be powered at 5 V (usually precise enough if the cable drop is
  neglected). The disadvantage is that some zener diodes have a lower voltage
  than 3 V when powered through 1k5 and the choice of components becomes
  relevant. In addition to that, the power consumption during USB data
  transfer is increased because the current is only limited by the 68 Ohm
  series resistor. The zeners may even distort the signal waveforms due to
  their capacity.

with-series-diodes.png and with-series-diodes.sch (EAGLE schematics):
  This is a simplified low-cost version of the voltage regulator approach.
  Instead of using a voltage regulator, we reduce the voltage by the forward
  voltage of two silicon diodes (roughly 1.4 V). This gives ca. 3.6 V which
  is practically inside the allowed range. The big disadvantage is that the
  supply is not regulated -- it even depends strongly on the power
  consumption. This cannot be tolerated for analog circuits.

tiny45-rc.png and tiny45-rc.sch (EAGLE schematics):
  This is mostly an example for connecting an 8 pin device using the internal
  RC oscillator for system clock. This example uses series diodes to limit
  the supply, but you may choose any other method. Please note that you must
  choose a clock rate of 12.8 or 16.5 MHz because only the receiver modules
  for these frequencies have a PLL to allow higher clock rate tolerances.


GENERAL DESIGN NOTES
====================
All examples have D+ on hardware interrupt INT0 because this is the highest
priority interrupt on AVRs. You may use other hardware interrupts (and
configure the options at the end of usbconfig.h accordingly) if you make sure
that no higher priority interrupt is used.

If you use USB_SOF_HOOK or USB_COUNT_SOF in usbconfig.h, you must wire D- to
the interrupt instead. This way the interrupt is triggered on USB Start Of
Frame pulses as well.

Most examples have a 1M pull-down resistor at D+. This pull-up ensures that
in self-powered designs no interrupts occur while USB is not connected. You
may omit this resistor in bus-powered designs. Older examples had a pull-up
resistor instead. This is not compatible with the zener diode approach to
level conversion: 1M pull-up in conjunction with a 3.6 V zener diode give an
invalid logic level.

All examples with ATMega8/88/168 have D+ at port D bit 2 (because this is
hardware interrupt 0) and D- on port D bit 4 because it is also a clock input
for timer/counter 0. This way the firmware can easily check for activity on
D- (USB frame pulses) by checking the counter value in regular intervals. If
no activity is found, the firmware should (according to the USB
specification) put the system into a low power suspend mode.



----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/

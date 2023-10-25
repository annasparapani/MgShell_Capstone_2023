# MgShell Capstone Project

> ###### Last update note - oct. 23
> After submitting the project in June 23, Anna Sparapani and Gianluca De Franceschi performed some additional optimization in October 2023. The updated code allointroduces bidirectional communication with Arduino and the sensor, which was not avaiable before. The communication protocol used is the following: 
<details> <summary> Protocol </summary>

1. the protocol manages the communication with the pH sensor (pH EZO by Atlas Scientific) with forwarding of eveything that is sent to Arduino through the PC (USB) serial port to the pH sensor. In this way the code allows to communicate with the pH sensor following its protocol (defined in the pH EZO board datasheet).

   > calibration can be performed with this code, single readings be taken when needed

2. switching on and off the automatic measure every "interval"ms and setting of the measure interval.  this is a custom protocol that'a been added to the code to allow more interaction. It includes: 
```'A,0'``` switches off the automatic reading
```'A,1'``` switches on the automatic reading
```'A,2,ms'``` switches on the automatic reading and sets the "interval" to a duration of "ms" milliseconds
   > be careful not to set a too short reading interval, the printing on the serial monitor won't keep up otherwise.
</details>

## Augmented Arduino Code for pH reading

This folder contains code by L. Bontempi, G. Noya, G. De Franceschi, M.Marras C. Russo and A. Sparapani. The original code for sensor reading was provided by MgShell.

Within the scope of the **Capstone Projects (PoliMi, 2023, professor Jacchetti)**, we were asked by MgShell to include a pH sensor in the current incubator setup. We developed two alternative solutions:

1. **Plan A**: adding a **pH microprobe by Atlas Scientific**, inserted into the eye dummy chamber through a threaded hole. This solution respects all requirements from MgShell.
2. **Plan B**: **flowloop** external to the eye dummy chamber and a **standard pH probe**. This was requested since the delivery times of the microprobe sensor requiresd for Plan A are longer than expected, so it's a temporary solution for the setup.
   > While being a temporary solution, the code and implementation leave MgShell with a functioning external flowloop that can be used for other applications if needed.
   >

In this folder you will find

1. [Code](/MgShell_Capstone_2023/arduino_UNO_pH_sample_code/arduino_UNO_pH_sample_code.ino) to read the pH sensor with Arduino provided by Atlas Scientfic
2. [Edited code](/mgshellCode_oct23/mgshellCode_oct23.ino) for Arduino control of the MgShell's setup

> This code is an updated version at oct. 2023 that allows complete interaction with the pH sensor directly from the serial monitor of the computer.
> The previous version submitted in June is found [here](/incubator_redone_2.0/incubator_redone_2.0.ino)

3. [Code to control the flowloop](/MgShell_Capstone_2023/main/main.ino) in case the option B - flowloop is implemented

They are both written in .ino format and can be built on an ArduinoUNO or MEGA.

We remind you that all contents of this folder are protected by an **Non Disclosure Agreement**, this is a private folder and its contents cannot be shared with people outside the project.

Last update: october 2023.

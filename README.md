# MgShell Capstone Project 
## Augmented Arduino Code for pH reading 
This folder contains code by L. Bontempi, G. Noya, G. De Franceschi, M.Marras C. Russo and A. Sparapani. The original code for sensor reading was provided by MgShell.

Within the scope of the **Capstone Projects (PoliMi, 2023, professor Jacchetti)**, we were asked by MgShell to include a pH sensor in the current incubator setup. We developed two alternative solutions:
1. **Plan A**: adding a **pH microprobe by Atlas Scientific**, inserted into the eye dummy chamber through a threaded hole. This solution respects all requirements from MgShell. 
2. **Plan B**: **flowloop** external to the eye dummy chamber and a **standard pH probe**. This was requested since the delivery times of the microprobe sensor requiresd for Plan A are longer than expected, so it's a temporary solution for the setup. 
    > While being a temporary solution, the code and implementation leave MgShell with a functioning external flowloop that can be used for other applications if needed. 

In this folder you will find
1. [Code](/MgShell_Capstone_2023/arduino_UNO_pH_sample_code/arduino_UNO_pH_sample_code.ino) to read the pH sensor with Arduino provided by Atlas Scientfic 
2. [Edited code](/MgShell_Capstone_2023/codiceincubatoriLTMP/codiceincubatoriLTMP.ino) for Arduino control of the MgShell's setup
3. [Code to control the flowloop](/MgShell_Capstone_2023/main/main.ino) in case the option B - flowloop is implemented

They are both written in .ino format and can be built on an ArduinoUNO or MEGA. 

We remind you that all contents of this folder are protected by an **Non Disclosure Agreement**, this is a private folder and its contents cannot be shared with people outside the project.

### To do
**Both Setups**
- [x] Code for reading the sensors
- [x] Code for saving data into the SD card 
- [x] Time the SD saving (once, twice a day?)

**Plan B - Flowloop**
- [x] Code for controlling the pump 
- [x] Code for controling the solenoid valve 
- [ ] All code together (sensing + flowloop control)

#ifndef BOOTVERSION_H
#define BOOTVERSION_H

#include <Arduino.h>
#include <Logger.h>
#include <HomeCommands.h>
#include <Devices.h>

// uint8_t _temp_ini = 0x0; // variabel used for ini process (!)

struct BOOTVERSION {
    uint8_t INI_DEVICE = _INI_1;
    uint8_t STATUS = DEVICE_ERROR;
    uint8_t x = 0;

    uint16_t CONTROLLER = MASTER_ADDRESS; // send all data to master first -> then to the real controller

    
};

BOOTVERSION BOOT;

bool calibrateBootversion() {
    if(DATA.COMMAND!=C_BOOTVERSION) {return false;}

    uint8_t cmd = DATA.D[0];

    switch(cmd) {

    case DEVICE_SETUP: {

        Logger::log("\tBootversion Conf");
        DATA.D[7] = DEVICE_OK;
        _temp_ini |= _INI_1;
        BOOT.STATUS = DEVICE_IDLE;

        DATA.setMasterPackage(CAN_SEND);
        break; }

    case DEVICE_CONNECT: { // Answer to master which wants to connect [Command to Master, Type,,,,,,Status]
        
        BOOT.CONTROLLER = DATA.SUBCOMMAND;

        BOOT.STATUS = DATA.D[4]; // ACTIVE, PASSIVE, IDLE
        
        DATA.D[0] = DEVICE_CONNECTED; // command to handle as answer
        DATA.D[1] = CORE.TYPE;        
        DATA.D[7] = DEVICE_OK;

        Logger::log("Connect to controller", BOOT.CONTROLLER, HEX);

        DATA.setPackage(CAN_SEND,BOOT.CONTROLLER,C_NCONT,CORE.ADDR);
        break; }

    case DEVICE_DISCONNECT: {

        BOOT.STATUS = DEVICE_IDLE;
        DATA.D[0] = DEVICE_DISCONNECTED;
        DATA.D[1] = CORE.TYPE;
        DATA.D[7] = DEVICE_OK;

        DATA.setPackage(CAN_SEND,BOOT.CONTROLLER,C_NCONT,CORE.ADDR);
        break;}

    default: {
        DATA.D[7] = DEVICE_ERROR; // last byte is OK\NOK byte
        Logger::log("\tBootversion Error");
        break;}
    }


    return true;
}

/**
 * @brief System relevant commands for adjusting the device -> not function specific (e.g. measure, set, ...). Can be used to specially configure the device type
 * This includes the corecommands which are shared among all devices
 * 
 * @return true 
 * @return false 
 */
bool overrideCommands() {

    if(calibrateBootversion()) {
        return true;
    }
  
    return false;
}


bool iniDevice() {

  if(CORE.INI_DEVICE!=CORE_OK) {
    if(DATA.CODE==CAN_DONE && CORE.ADDR==DATA.ADDRESS) {

        overrideCommands();
        initializeDevice(_temp_ini == BOOT.INI_DEVICE);
    }
  }

  if(CORE.INI_DEVICE==CORE_OK) {
    return true;
  }

  return false;
}








#endif 
#include <Arduino.h>

#include <Logger.h>
  bool DEBUGLOG=true;
  
#include <Core_CAN.h>
#include <Corecommands_CAN.h>

#include <Bootversion.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Logger::log("--- Bootversion ---");
  CORE.TYPE=T_BOOTVERSION;

  // load bus
  if(!CAN.begin(CAN_500KBPS)) {    
    Logger::log("\tCAN has errors");
    CORE.RESIDUAL = CORE_FAIL;
  } 

  CORE.checkResidual();
  // CORE.checkResidualDebug();

  Logger::log("\tWaiting for residual");


}

void calculationLoop() {
  Logger::log("calculation");
}

/**
 * @brief Put main function of the subunit here
 * 
 */
void normalLoop() { // do stuff from interrupt CAN-data or CAN-data during idle time
  
  // DATA.log_msg("NORMAL");

  bool normalreset = true;

  if(DATA.COMMAND==C_MEASURE || DATA.COMMAND==C_NORMAL) { // normal operation 
    Logger::log("normal measure");
    CORE.resetPowerTimerInterrupt();
    normalreset=false;
  } else if(DATA.COMMAND==C_ACKNOWLEDGE)  {
    Logger::log("Acknowledge");
  } else {
    Logger::log("normal empty");
  }

  if(normalreset) {
    CORE.resetPowerTimer();
  }

  DATA.clear();
}

void wakeupLoop() { // use for standard operation (active, passive operation) // has no inherit DATA
  Logger::log("wakeup");

  DATA.setPackage(CAN_DONE,CORE.ADDR,C_NORMAL,C_NORMAL);

  normalLoop();
}

void interruptLoop() { // holds DATA
  // DATA.log_msg("interrupt");
  normalLoop();
}

/**
 * @brief Continious running function inbetween Wakeup and Sleep (after interrupt).
 * Here all incoming messages should be caught, before going to sleep again (resetPowerTimer). When a new message arrives, the normal procedure will be called
 * 
 */
void idleLoop() { // 
  if(checkForData()) { // 
    normalLoop();
  }
}


void loop() {

  scanBus();

  if(overrideCoreCommands()) { // essentials commands of the operating systems
    // Logger::log("Override_Corecommands");
    CORE.resetPowerTimer();
    // CORE.interrupt();
    // CORE.idle();
  } else if(iniLoop() && iniDevice()) {
  // } else if(iniDebugLoop() && iniDevice()) {
  // } else if(iniDebugLoop() && iniDebugDevice()) {
    if(overrideCommands()) {
      // CORE_POWER_STATUS = CORE_POWER_NORMAL;
      Logger::log("Override_Commands");
      CORE.resetPowerTimer();
      // CORE.interrupt();
      // CORE.idle();

    } else { 

      switch(CORE_POWER_STATUS) {

        case CORE_POWER_WAKEUP: {
          wakeupLoop();
          break; }
        case CORE_POWER_INTERRUPT: {
          interruptLoop();
          break;}
        case CORE_POWER_CALCULATION: {
          calculationLoop();
          break;}
        case CORE_POWER_IDLE: { // not defined bustelegrams 
          idleLoop();     
          break;}
        case CORE_POWER_NORMAL: {
          normalLoop();
          // CORE_POWER_STATUS = CORE_POWER_IDLE;
          break;}
        default: {
            Logger::log("\tERROR");
            break; }

      }
      
    }
    
    CORE_POWER_STATUS = core_lowPowerMode_TimerInterruptCAN();
      
    
  }

  transmitBus();
}

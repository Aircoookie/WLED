/* CAN OBD & UDS Simulator
 *  
 *  Currently replies to some general OBD requests 
 *  Place holders exist for what I've been able to locate on the Internet
 *  Does not currently support UDS correctly, just placeholders with mode descriptions
 *  
 *  Written By: Cory J. Fowler  December 20th, 2016
 *  
 */ 

#include <mcp_can.h>
#include <SPI.h>


#define PAD 0x00

// What CAN ID type?  Standard or Extended
#define standard 0

// 7E0/8 = Engine ECM
// 7E1/9 = Transmission ECM

#if standard == 1
  #define REPLY_ID 0x7E9
  #define LISTEN_ID 0x7E1
  #define FUNCTIONAL_ID 0x7DF  
#else
  #define REPLY_ID 0x98DAF101
  #define LISTEN_ID 0x98DA01F1
  #define FUNCTIONAL_ID 0x98DB33F1
#endif


// CAN RX Variables
unsigned long rxId;
byte dlc;
byte rxBuf[8];

// CAN Interrupt and Chip Select
#define CAN0_INT 2                              // Set CAN0 INT to pin 2
MCP_CAN CAN0(9);                                // Set CAN0 CS to pin 9


void setup()
{
  Serial.begin(115200);
  while(!Serial);
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_20MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");


#if standard == 1
  // Standard ID Filters
  CAN0.init_Mask(0,0x7F00000);                // Init first mask...
  CAN0.init_Filt(0,0x7DF0000);                // Init first filter...
  CAN0.init_Filt(1,0x7E10000);                // Init second filter...
  
  CAN0.init_Mask(1,0x7F00000);                // Init second mask... 
  CAN0.init_Filt(2,0x7DF0000);                // Init third filter...
  CAN0.init_Filt(3,0x7E10000);                // Init fourth filter...
  CAN0.init_Filt(4,0x7DF0000);                // Init fifth filter...
  CAN0.init_Filt(5,0x7E10000);                // Init sixth filter...

#else
  // Extended ID Filters
  CAN0.init_Mask(0,0x90FFFF00);                // Init first mask...
  CAN0.init_Filt(0,0x90DB3300);                // Init first filter...
  CAN0.init_Filt(1,0x90DA0100);                // Init second filter...
  
  CAN0.init_Mask(1,0x90FFFF00);                // Init second mask... 
  CAN0.init_Filt(2,0x90DB3300);                // Init third filter...
  CAN0.init_Filt(3,0x90DA0100);                // Init fourth filter...
  CAN0.init_Filt(4,0x90DB3300);                // Init fifth filter...
  CAN0.init_Filt(5,0x90DA0100);                // Init sixth filter...
#endif
  
  CAN0.setMode(MCP_NORMAL);                          // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input
  
  Serial.println("OBD-II CAN Simulator");
}

void loop()
{
  if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &dlc, rxBuf);             // Get CAN data
    
    // First request from most adapters...
    if(rxId == FUNCTIONAL_ID){
      obdReq(rxBuf);
    }       
  }
}


void obdReq(byte *data){
  byte numofBytes = data[0];
  byte mode = data[1] & 0x0F;
  byte pid = data[2];
  bool tx = false;
  byte txData[] = {0x00,(0x40 | mode),pid,PAD,PAD,PAD,PAD,PAD};

  
  //txData[1] = 0x40 | mode;
  //txData[2] = pid; 
  
  //=============================================================================
  // MODE $01 - Show current data
  //=============================================================================
  if(mode == 0x01){
    if(pid == 0x00){        // Supported PIDs 01-20
      txData[0] = 0x06;
      
      txData[3] = 0x80;
      txData[4] = 0x38;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
    }
    else if(pid == 0x01){    // Monitor status since DTs cleared.
      bool MIL = true;
      byte DTC = 5;
      txData[0] = 0x06;
      
      txData[3] = (MIL << 7) | (DTC & 0x7F);
      txData[4] = 0x07;
      txData[5] = 0xFF;
      txData[6] = 0x00;
      tx = true;
    }
//    else if(pid == 0x02){    // Freeze DTC
//    }
    else if(pid == 0x03){    // Fuel system status
      txData[0] = 0x03;
      
      txData[3] = 0xFA;
      tx = true;
    }
//    else if(pid == 0x04){    // Calculated engine load
//    }
    else if(pid == 0x05){    // Engine coolant temperature
      txData[0] = 0x03;
      
      txData[3] = 0xFA;
      tx = true;
    }
//    else if(pid == 0x06){    // Short term fuel trim - Bank 1
//    }
//    else if(pid == 0x07){    // Long tern fuel trim - Bank 1
//    }
//    else if(pid == 0x08){    // Short term fuel trim - Bank 2
//    }
//    else if(pid == 0x09){    // Long term fuel trim - Bank 2
//    }
//    else if(pid == 0x0A){    // Fuel pressure (gauge)
//    }
    else if(pid == 0x0B){    // Intake manifold absolute pressure
      txData[0] = 0x03;
      
      txData[3] = 0x64;
      tx = true;
    }
    else if(pid == 0x0C){    // Engine RPM
      txData[0] = 0x04;
      
      txData[3] = 0x9C;
      txData[4] = 0x40;
      tx = true;
    }
    else if(pid == 0x0D){    // Vehicle speed
      txData[0] = 0x03;
      
      txData[3] = 0xFA;
      tx = true;
    }
//    else if(pid == 0x0E){    // Timing advance
//    }
    else if(pid == 0x0F){    // Intake air temperature
      txData[0] = 0x03;
      
      txData[3] = 0xFA;
      tx = true;
    }
//    else if(pid == 0x10){    // MAF air flow rate
//    }
    else if(pid == 0x11){    // Throttle position
      txData[0] = 0x03;
      
      txData[3] = 0xFA;
      tx = true;
    }
//    else if(pid == 0x12){    // Commanded secondary air status
//    }
//    else if(pid == 0x13){    // Oxygen sensors present (in 2 banks)
//    }
//    else if(pid == 0x14){    // Oxygen Sensor 1 (Voltage & Trim)
//    }
//    else if(pid == 0x15){    // Oxygen Sensor 2 (Voltage & Trim)
//    }
//    else if(pid == 0x16){    // Oxygen Sensor 3 (Voltage & Trim)
//    }
//    else if(pid == 0x17){    // Oxygen Sensor 4 (Voltage & Trim)
//    }
//    else if(pid == 0x18){    // Oxygen Sensor 5 (Voltage & Trim)
//    }
//    else if(pid == 0x19){    // Oxygen Sensor 6 (Voltage & Trim)
//    }
//    else if(pid == 0x1A){    // Oxygen Sensor 7 (Voltage & Trim)
//    }
//    else if(pid == 0x1B){    // Oxygen Sensor 8 (Voltage & Trim)
//    }
//    else if(pid == 0x1C){    // OBD standards this vehicle conforms to
//    }
//    else if(pid == 0x1D){    // Oxygen sensors present (in 4 banks)
//    }
//    else if(pid == 0x1E){    // Auxiliary input status
//    }
//    else if(pid == 0x1F){    // Run time since engine start
//    }
    else if(pid == 0x20){    // Supported PIDs 21-40
      txData[0] = 0x06;
      
      txData[3] = 0x80;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
    }
    else if(pid == 0x21){    // Distance traveled with MIL on
      txData[0] = 0x04;
      
      txData[3] = 0x00;
      txData[4] = 0x23;
      tx = true;
    }
//    else if(pid == 0x22){    // Fuel rail pressure (Relative to Manifold Vacuum)
//    }
//    else if(pid == 0x23){    // Fuel rail gauge pressure (diesel or gasoline direct injection)
//    }
//    else if(pid == 0x24){    // Oxygen Sensor 1 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x25){    // Oxygen Sensor 2 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x26){    // Oxygen Sensor 3 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x27){    // Oxygen Sensor 4 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x28){    // Oxygen Sensor 5 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x29){    // Oxygen Sensor 6 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x2A){    // Oxygen Sensor 7 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x2B){    // Oxygen Sensor 8 (Fuel to Air & Voltage)
//    }
//    else if(pid == 0x2C){    // Commanded EGR
//    }
//    else if(pid == 0x2D){    // EGR Error
//    }
//    else if(pid == 0x2E){    // Commanded evaporative purge
//    }
//    else if(pid == 0x2F){    // Fuel tank level input
//    }
//    else if(pid == 0x30){    // Warm-ups since codes cleared
//    }
//    else if(pid == 0x31){    // Distance traveled since codes cleared
//    }
//    else if(pid == 0x32){    // Evap. System Vapor Pressure
//    }
//    else if(pid == 0x33){    // Absolute Barometric Pressure
//    }
//    else if(pid == 0x34){    // Oxygen Sensor 1 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x35){    // Oxygen Sensor 2 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x36){    // Oxygen Sensor 3 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x37){    // Oxygen Sensor 4 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x38){    // Oxygen Sensor 5 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x39){    // Oxygen Sensor 6 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x3A){    // Oxygen Sensor 7 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x3B){    // Oxygen Sensor 8 (Fuel to Air & Current) 
//    }
//    else if(pid == 0x3C){    // Catalyst Temperature: Bank 1, Sensor 1
//    }
//    else if(pid == 0x3D){    // Catalyst Temperature: Bank 2, Sensor 1
//    }
//    else if(pid == 0x3E){    // Catalyst Temperature: Bank 1, Sensor 2
//    }
//    else if(pid == 0x3F){    // Catalyst Temperature: Bank 2, Sensor 2
//    }
    else if(pid == 0x40){    // Supported PIDs 41-60
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x08;
      txData[5] = 0x00;
      txData[6] = 0x0D;
      tx = true;
    }
//    else if(pid == 0x41){    // Monitor status this drive cycle
//    }
//    else if(pid == 0x42){    // Control module voltage
//    }
//    else if(pid == 0x43){    // Absolute load value
//    }
//    else if(pid == 0x44){    // Fuel-Air commanded equivalence ratio
//    }
//    else if(pid == 0x45){    // Relative throttle position
//    }
//    else if(pid == 0x46){    // Ambient air temperature
//    }
//    else if(pid == 0x47){    // Absolute throttle position B
//    }
//    else if(pid == 0x48){    // Absolute throttle position C
//    }
//    else if(pid == 0x49){    // Accelerator pedal position D
//    }
//    else if(pid == 0x4A){    // Accelerator pedal position E
//    }
//    else if(pid == 0x4B){    // Accelerator pedal position F
//    }
//    else if(pid == 0x4C){    // Commanded throttle actuator
//    }
    else if(pid == 0x4D){    // Time run with MIL on
      txData[0] = 0x04;
      
      txData[3] = 0x00;
      txData[4] = 0x3C;
      tx = true;
    }
//    else if(pid == 0x4E){    // Time since troble codes cleared
//    }
//    else if(pid == 0x4F){    // Time since trouble codes cleared
//    }
//    else if(pid == 0x50){    // Maximum value for Fuel-Air equivalence ratio, oxygen sensor voltage, oxygen sensro current, and intake manifold absolute-pressure
//    }
//    else if(pid == 0x51){    // Fuel Type
//    }
//    else if(pid == 0x52){    // Ethanol Fuel %
//    }
//    else if(pid == 0x53){    // Absolute evap system vapor pressure
//    }
//    else if(pid == 0x54){    // Evap system vapor pressure
//    }
//    else if(pid == 0x55){    // Short term secondary oxygen sensor trim, A: bank 1, B: bank 3
//    }
//    else if(pid == 0x56){    // Long term secondary oxygen sensor trim, A: bank 1, B: bank 3
//    }
//    else if(pid == 0x57){    // Short term secondary oxygen sensor trim, A: bank 2, B: bank 4
//    }
//    else if(pid == 0x58){    // Long term secondary oxygen sensor trim, A: bank 2, B: bank 4
//    }
//    else if(pid == 0x59){    // Fuel rail absolute pressure
//    }
//    else if(pid == 0x5A){    // Relative accelerator pedal position
//    }
//    else if(pid == 0x5B){    // Hybrid battery pack remaining life
//    }
    else if(pid == 0x5C){    // Engine oil Temperature
      txData[0] = 0x03;
      
      txData[3] = 0x1E;
      tx = true;
    }
    else if(pid == 0x5D){    // Fuel injection timing
      txData[0] = 0x04;
      
      txData[3] = 0x61;
      txData[4] = 0x80;
      tx = true;
    }
    else if(pid == 0x5E){    // Engine fuel rate
      txData[0] = 0x04;
      
      txData[3] = 0x07;
      txData[4] = 0xD0;
      tx = true;
    }
//    else if(pid == 0x5F){    // Emissions requirements to which vehicle is designed
//    }
    else if(pid == 0x60){    // Supported PIDs 61-80
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
    }
//    else if(pid == 0x61){    // Driver's demand engine - percent torque
//    }
//    else if(pid == 0x62){    // Actual engine - percent torque
//    }
//    else if(pid == 0x63){    // Engine reference torque
//    }
//    else if(pid == 0x64){    // Engine percent torque data
//    }
//    else if(pid == 0x65){    // Auxiliary input / output supported
//    }
//    else if(pid == 0x66){    // Mas air flow sensor
//    }
//    else if(pid == 0x67){    // Engine coolant temperature
//    }
//    else if(pid == 0x68){    // Intake air temperature sensor
//    }
//    else if(pid == 0x69){    // Commanded EGR and EGR error
//    }
//    else if(pid == 0x6A){    // Commanded Diesel intake air flow control and relative intake air flow position
//    }
//    else if(pid == 0x6B){    // Exhaust gas recirculation temperature
//    }
//    else if(pid == 0x6C){    // Commanded throttle actuator control and relative throttle position
//    }
//    else if(pid == 0x6D){    // Fuel pressure control system
//    }
//    else if(pid == 0x6E){    // Injection pressure control system
//    }
//    else if(pid == 0x6F){    // Turbocharger compressor inlet pressure
//    }
//    else if(pid == 0x70){    // Boost pressure control
//    }
//    else if(pid == 0x71){    // Variable Geometry turbo sensor
//    }
//    else if(pid == 0x72){    // Wastegate control
//    }  
//    else if(pid == 0x73){    // Exhaust pressure
//    }
//    else if(pid == 0x74){    // Turbocharger RPM
//    }
//    else if(pid == 0x75){    // Turbocharger temperature
//    }
//    else if(pid == 0x76){    // Turbocharger temperature
//    }
//    else if(pid == 0x77){    // Charge air cooler temperature (CACT)
//    }
//    else if(pid == 0x78){    // Exhaust Gas Temperature (EGT) bank 1
//    }
//    else if(pid == 0x79){    // Exhaust Gas Temperature (EGT) bank 2
//    }
//    else if(pid == 0x7A){    // Diesel particulate filter (DPF)
//    }
//    else if(pid == 0x7B){    // Diesel particulate filter (DPF)
//    }
//    else if(pid == 0x7C){    // Diesel particulate filter (DPF) temperature
//    }
//    else if(pid == 0x7D){    // NOx NTE control area status
//    }
//    else if(pid == 0x7E){    // PM NTE control area status
//    }
//    else if(pid == 0x7F){    // Engine run time
//    }
    else if(pid == 0x80){    // Supported PIDs 81-A0
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
    }
//    else if(pid == 0x81){    // Engine run time for Auxiliary Emissions Control Device (AECD)
//    }
//    else if(pid == 0x82){    // Engine run time for Auxiliary Emissions Control Device (AECD)
//    }  
//    else if(pid == 0x83){    // NOx sensor
//    }
//    else if(pid == 0x84){    // Manifold surface temperature
//    }
//    else if(pid == 0x85){    // NOx reqgent system
//    }
//    else if(pid == 0x86){    // Particulate Matter (PM) sensor
//    }
//    else if(pid == 0x87){    // Intake manifold absolute pressure
//    }
    else if(pid == 0xA0){    // Supported PIDs A1-C0
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
    }
    else if(pid == 0xC0){    // Supported PIDs C1-E0
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
    }
    else if(pid == 0xE0){    // Supported PIDs E1-FF?
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x00;
      tx = true;
    }
    else{
      unsupported(mode, pid);
    }
  }
  
  //=============================================================================
  // MODE $02 - Show freeze frame data
  //=============================================================================
  else if(mode == 0x02){
      unsupported(mode, pid);
  }
  
  //=============================================================================
  // MODE $03 - Show stored DTCs
  //=============================================================================
  else if(mode == 0x03){
      byte DTCs[] = {(0x40 | mode), 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      iso_tp(mode, pid, 12, DTCs);
  }
  
  //=============================================================================
  // MODE $04 - Clear DTCs and stored values
  //=============================================================================
  else if(mode == 0x04){
      // Need to cleat DTCs.  We just acknowledge the command for now.
      txData[0] = 0x01;
      tx = true;
  }
  
  //=============================================================================
  // MODE $05 - Test Results, oxygen sensor monitoring (non CAN only)
  //=============================================================================
  else if(mode == 0x05){
      unsupported(mode, pid);
  }
  
  //=============================================================================
  // MODE $06 - Test Results, On-Board Monitoring (Oxygen sensor monitoring for CAN only)
  //=============================================================================
  else if(mode == 0x06){
    if(pid == 0x00){        // Supported PIDs 01-20
      txData[0] = 0x06;
      
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x00;
      tx = true;
    }
    else{
      unsupported(mode, pid);
    }
  }
  
  //=============================================================================
  // MODE $07 - Show pending DTCs (Detected during current or last driving cycle)
  //=============================================================================
  else if(mode == 0x07){
      byte DTCs[] = {(0x40 | mode), 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      iso_tp(mode, pid, 12, DTCs);
  }
  
  //=============================================================================
  // MODE $08 - Control operation of on-board component/system
  //=============================================================================
  else if(mode == 0x08){
      unsupported(mode, pid);
  }
  
  //=============================================================================
  // MODE $09 - Request vehcile information
  //=============================================================================
  else if(mode == 0x09){
    if(pid == 0x00){        // Supported PIDs 01-20
      txData[0] = 0x06;
      
      txData[3] = 0x54;
      txData[4] = 0x40;
      txData[5] = 0x00;
      txData[6] = 0x00;
      tx = true;
    }
//    else if(pid == 0x01){    // VIN message count for PID 02. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
//    }
    else if(pid == 0x02){    // VIN (17 to 20 Bytes) Uses ISO-TP
      byte VIN[] = {(0x40 | mode), pid, 0x01, 0x31, 0x5a, 0x56, 0x42, 0x50, 0x38, 0x41, 0x4d, 0x37, 0x44, 0x35, 0x32, 0x32, 0x30, 0x31, 0x38, 0x31};
      iso_tp(mode, pid, 20, VIN);
    }
//    else if(pid == 0x03){    // Calibration ID message count for PID 04. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
//    }
    else if(pid == 0x04){    // Calibration ID
      byte CID[] = {(0x40 | mode), pid, 0x01, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x20, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D, 0x51, 0x52, 0x53, 0x54};
      iso_tp(mode, pid, 23, CID);
    }
//    else if(pid == 0x05){    // Calibration Verification Number (CVN) message count for PID 06. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
//    }
    else if(pid == 0x06){    // CVN
      byte CVN[] = {(0x40 | mode), pid, 0x02, 0x11, 0x42, 0x42, 0x42, 0x22, 0x43, 0x43, 0x43};
      iso_tp(mode, pid, 11, CVN);
    }
//    else if(pid == 0x07){    // In-use performance tracking message count for PID 08 and 0B. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
//    }
//    else if(pid == 0x08){    // In-use performance tracking for spark ignition vehicles.
//    }
//    else if(pid == 0x09){    // ECU name message count for PID 0A.
//    }
    else if(pid == 0x0A){    // ECM Name
      byte ECMname[] = {(0x40 | mode), pid, 0x01, 0x45, 0x43, 0x4D, 0x00, 0x2D, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D};
      iso_tp(mode, pid, 23, ECMname);
    }
//    else if(pid == 0x0B){    // In-use performance tracking for compression ignition vehicles.
//    }
//    else if(pid == 0x0C){    // ESN message count for PID 0D.
//    }
    else if(pid == 0x0D){    // ESN
      byte ESN[] = {(0x40 | mode), pid, 0x01, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x2D, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D, 0x00};
      iso_tp(mode, pid, 20, ESN);
    }
    else{
      unsupported(mode, pid); 
    }
  }
  
  //=============================================================================
  // MODE $0A - Show permanent DTCs 
  //=============================================================================
  else if(mode == 0x0A){
      byte DTCs[] = {(0x40 | mode), 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      iso_tp(mode, pid, 12, DTCs);
  }
  
  // UDS Modes: Diagonstic and Communications Management =======================================
  //=============================================================================
  // MODE $10 - Diagnostic Session Control
  //=============================================================================
//  else if(mode == 0x10){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $11 - ECU Reset
  //=============================================================================
//  else if(mode == 0x11){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $27 - Security Access
  //=============================================================================
//  else if(mode == 0x27){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $28 - Communication Control
  //=============================================================================
//  else if(mode == 0x28){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $3E - Tester Present
  //=============================================================================
//  else if(mode == 0x3E){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $83 - Access Timing Parameters
  //=============================================================================
//  else if(mode == 0x83){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $84 - Secured Data Transmission
  //=============================================================================
//  else if(mode == 0x84){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
//  
  //=============================================================================
  // MODE $85 - Control DTC Sentings
  //=============================================================================
//  else if(mode == 0x85){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $86 - Response On Event
  //=============================================================================
//  else if(mode == 0x86){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $87 - Link Control
  //=============================================================================
//  else if(mode == 0x87){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  // UDS Modes: Data Transmission ==============================================================
  //=============================================================================
  // MODE $22 - Read Data By Identifier
  //=============================================================================
//  else if(mode == 0x22){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $23 - Read Memory By Address
  //=============================================================================
//  else if(mode == 0x23){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $24 - Read Scaling Data By Identifier
  //=============================================================================
//  else if(mode == 0x24){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $2A - Read Data By Periodic Identifier
  //=============================================================================
//  else if(mode == 0x2A){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $2C - Dynamically Define Data Identifier
  //=============================================================================
//  else if(mode == 0x2C){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $2E - Write Data By Identifier
  //=============================================================================
//  else if(mode == 0x2E){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $3D - Write Memory By Address
  //=============================================================================
//  else if(mode == 0x3D){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  // UDS Modes: Stored Data Transmission =======================================================
  //=============================================================================
  // MODE $14 - Clear Diagnostic Information
  //=============================================================================
//  else if(mode == 0x14){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $19 - Read DTC Information
  //=============================================================================
//  else if(mode == 0x19){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      tx = true;
//  }
  
  // UDS Modes: Input Output Control ===========================================================
  //=============================================================================
  // MODE $2F - Input Output Control By Identifier
  //=============================================================================
//  else if(mode == 0x2F){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  // UDS Modes: Remote Activation of Routine ===================================================
  //=============================================================================
  // MODE $31 - Routine Control
  //=============================================================================
//  else if(mode == 0x2F){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  // UDS Modes: Upload / Download ==============================================================
  //=============================================================================
  // MODE $34 - Request Download
  //=============================================================================
//  else if(mode == 0x34){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $35 - Request Upload
  //=============================================================================
//  else if(mode == 0x35){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $36 - Transfer Data
  //=============================================================================
//  else if(mode == 0x36){
//      txData[0] = 0x02;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $37 - Request Transfer Exit
  //=============================================================================
//  else if(mode == 0x37){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  
  //=============================================================================
  // MODE $38 - Request File Transfer
  //=============================================================================
//  else if(mode == 0x38){
//      txData[0] = 0x03;
//      
//      txData[2] = mode;
//      txData[3] = 0x00;
//      tx = true;
//  }
  else { 
    unsupported(mode, pid);
  }
  
  if(tx)
    CAN0.sendMsgBuf(REPLY_ID, 8, txData);
}


// Generic debug serial output
void unsupported(byte mode, byte pid){
  negAck(mode, 0x12);
  unsupportedPrint(mode, pid);  
}


// Generic debug serial output
void negAck(byte mode, byte reason){
  byte txData[] = {0x03,0x7F,mode,reason,PAD,PAD,PAD,PAD};
  CAN0.sendMsgBuf(REPLY_ID, 8, txData);
}


// Generic debug serial output
void unsupportedPrint(byte mode, byte pid){
  char msgstring[64];
  sprintf(msgstring, "Mode $%02X: Unsupported PID $%02X requested!", mode, pid);
  Serial.println(msgstring);
}


// Blocking example of ISO transport
void iso_tp(byte mode, byte pid, int len, byte *data){
  byte tpData[8];
  int offset = 0;
  byte index = 0;
//  byte packetcnt = ((len & 0x0FFF) - 6) / 7;
//  if((((len & 0x0FFF) - 6) % 7) > 0)
//    packetcnt++;

  // First frame
  tpData[0] = 0x10 | ((len >> 8) & 0x0F);
  tpData[1] = 0x00FF & len;
  for(byte i=2; i<8; i++){
    tpData[i] = data[offset++];
  }
  CAN0.sendMsgBuf(REPLY_ID, 8, tpData);
  index++; // We sent a packet so increase our index.
  
  bool not_done = true;
  unsigned long sepPrev = millis();
  byte sepInvl = 0;
  byte frames = 0;
  bool lockout = false;
  while(not_done){
    // Need to wait for flow frame
    if(!digitalRead(CAN0_INT)){
      CAN0.readMsgBuf(&rxId, &dlc, rxBuf);
    
      if((rxId == LISTEN_ID) && ((rxBuf[0] & 0xF0) == 0x30)){
        if((rxBuf[0] & 0x0F) == 0x00){
          // Continue
          frames = rxBuf[1];
          sepInvl = rxBuf[2];
          lockout = true;
        } else if((rxBuf[0] & 0x0F) == 0x01){
          // Wait
          lockout = false;
          delay(rxBuf[2]);
        } else if((rxBuf[0] & 0x0F) == 0x03){
          // Abort
          not_done = false;
          return;
        }
      }
    }

    if(((millis() - sepPrev) >= sepInvl) && lockout){
      sepPrev = millis();

      tpData[0] = 0x20 | index++;
      for(byte i=1; i<8; i++){
        if(offset != len)
          tpData[i] = data[offset++];
        else
          tpData[i] = 0x00;
      }
      
      // Do consecutive frames as instructed via flow frame
      CAN0.sendMsgBuf(REPLY_ID, 8, tpData);
      
      if(frames-- == 1)
        lockout = false;
        
    }

    if(offset == len)
      not_done = false;
    else{
      char msgstring[32];
      sprintf(msgstring,"Offset: 0x%04X\tLen: 0x%04X", offset, len);
      Serial.println(msgstring);
    }


    // Timeout
    if((millis() - sepPrev) >= 1000)
      not_done = false;
  }
  
}
  

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

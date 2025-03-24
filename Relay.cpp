#include "Relay.h"

std::string Relay::convertPin(const Input& input, const Relay& relay){
    //Declare and initialize necessary variables
    std::string relayName = input.relayList[relay.currentRelay].name[0];
    std::string pin;
    int pinNumber;
    char lastChar = relayName.back();

    //Finds the pin number based on the last character of the name
    if(isdigit(lastChar)){
        //Converts the pin number to an integer
        pinNumber = lastChar - '0';

        //Checks if the pin number is a double digit number
        lastChar = relayName[relayName.size() - 2];
        if(isdigit(lastChar)){
            //Adjusts the pin number to the correct value
            pinNumber += (lastChar - '0') * 10;
        }

        //Converts the pin number to a string
        pin = "DUT_PIN_" + std::to_string(pinNumber);
    }
    else{
        //If the end of the name is not a number, it is set to the deafult pin
        pin = "DUT_PIN_1";
    }

    //Returns the pin number
    return pin;
}

std::string Relay::convertEnum(const Input& input, const Relay& relay){
    //Declare and initialize necessary variables
    std::string relayName = input.relayList[relay.currentRelay].name[0];
    std::string enumeration;
    std::vector<int> nameBreaks;
    char lastChar = relayName.back();
    int tracker = -1;

    //Finds if the last character of the name is a number
    if(isdigit(lastChar)){
        //Finds all instances of the underscore in the name
        while((tracker = relayName.find("_", tracker + 1)) != std::string::npos){
                nameBreaks.push_back(tracker);
        }
        //Replaces the middle information with CIB
        enumeration = relayName.substr(0, nameBreaks[0]+1) + "CIB" + relayName.substr(nameBreaks.back(), relayName.back()-nameBreaks.back()+1);
    }
    else{
        //If the end of the name is not a number, the final word is replaced with CIB unless it is a gnd
        if(relayName.find("SYSGND") != std::string::npos){
            enumeration = relayName.substr(0, relayName.find("SYSGND")) + "SYSGND";
        }
        else{
            //Finds all instances of the underscore in the name
            while((tracker = relayName.find("_", tracker + 1)) != std::string::npos){
                nameBreaks.push_back(tracker);
            }
            //Replaces the final word with CIB
            enumeration = relayName.substr(0, nameBreaks.back()+1) + "CIB";
        }
        
    }

    //Returns the enumeration
    return enumeration;
}

int Relay::convertChain(const Input& input){
    //Declare and initialize necessary variables
    int chainNumber;

    //Determines the chain number based on the board type
    if(input.boardType == "Power")
        chainNumber = 0;
    else if(input.boardType == "Precision")
        chainNumber = 1;

    //Returns the chain number
    return chainNumber;
}

int Relay::convertDevice(const Input& input, const Relay& relay){
    //Declare and initialize necessary variables
    int deviceNumber;
    
    //Determines which line of the netlist data has the device number
    for(int i=0; i<input.relayList[currentRelay].device.size(); i++){
        //Find the index of the connection to the IO expander
        if(input.relayList[currentRelay].device[i] == "MAX6957ATL_TQFN-40-MAX6957ATL+"){
            //Loops through each IO expander
            for(int j=0; j<input.ioData.size(); j++){
                //Checks which IO expander is associated with this relay
                if(input.ioData[j].identifier == input.relayList[currentRelay].identifier[i]){
                    deviceNumber = input.ioData[j].position + relay.deviceModifier;
                    break;
                }
            }
            break;
        }
    }
    
    //Returns the device number
    return deviceNumber;
}

std::string Relay::convertPort(const Input& input, const Relay& relay){
    //Declare and initialize necessary variables
    std::string portIdentifier;
    int connectionNumber = 0;
    std::string portNumber;
    
    //Determines which line of the netlist data has the port connection
    for(int i=0; i<input.relayList[currentRelay].device.size(); i++){
        //Find the index of the connection to the IO expander
        if(input.relayList[currentRelay].device[i] == "MAX6957ATL_TQFN-40-MAX6957ATL+"){
            //Assigns the port identifier based on connection to the IO expander
            portIdentifier = input.relayList[currentRelay].connection[i];
            break;
        }
    }

    //Converts the port identifier to the corresponding connection number
    connectionNumber = stoi(portIdentifier);

    //Determines the port number based on the connection number and returns it
    switch(connectionNumber){
        case 1:
            portNumber = "P8";
            break;
        case 2:
            portNumber = "P12";
            break;
        case 3:
            portNumber = "P9";
            break;
        case 4:
            portNumber = "P13";
            break;
        case 5:
            portNumber = "P10";
            break;
        case 6:
            portNumber = "P14";
            break;
        case 7:
            portNumber = "P11";
            break;
        case 8:
            portNumber = "P15";
            break;
        case 9:
            portNumber = "P16";
            break;
        case 10:
            portNumber = "P17";
            break;
        case 12:
            portNumber = "P18";
            break;
        case 13:
            portNumber = "P19";
            break;
        case 14:
            portNumber = "P20";
            break;
        case 15:
            portNumber = "P21";
            break;
        case 16:
            portNumber = "P22";
            break;
        case 17:
            portNumber = "P23";
            break;
        case 18:
            portNumber = "P24";
            break;
        case 19:
            portNumber = "P25";
            break;
        case 21:
            portNumber = "P26";
            break;
        case 22:
            portNumber = "P27";
            break;
        case 23:
            portNumber = "P28";
            break;
        case 24:
            portNumber = "P7";
            break;
        case 25:
            portNumber = "P29";
            break;
        case 26:
            portNumber = "P6";
            break;
        case 27:
            portNumber = "P30";
            break;
        case 28:
            portNumber = "P5";
            break;
        case 29:
            portNumber = "P31";
            break;
        case 30:
            portNumber = "P4";
            break;
    }
    return portNumber;
}

std::string Relay::convertDriveStrength(const Input& input, const Relay& relay){
    //Declare and initialize necessary variable
    std::string driveStrength = "CONFIG_OUT";   //The default drive strength is CONFIG_OUT, will change with a relay connection
    bool presentRelay = false;

    //Loops through all of the connection devices to see if a relay is present
    for(int i=0; i<input.relayList[currentRelay].device.size(); i++){
        //Checks to see if there is a relay on this connection
        for(int j=0; j<input.modelData.size(); j++){
            if(input.relayList[currentRelay].device[i].find(input.modelData[j].relayModel) != std::string::npos){
                driveStrength = input.modelData[j].driveStrength;
                break;
            }
        }
        //Breaks out of the loop if the drive strength has been found
        if(driveStrength != "CONFIG_OUT")
        break;
    }

    return driveStrength;
}

std::string Relay::convertNormalState(const Input& input, const Relay& relay){
    //Declare and initialize necessary variables, relays are normally open by default
    std::string normalState = "RELAY_NO";
    std::string resistorIdentifier = "";
    int tracker = 0;
    bool gndConnection = false;
    bool presentRelay = false;
    
    //Checks if any of the identifiers specify the relay is normally closed
    for(int i=0; i<input.relayList[currentRelay].device.size(); i++){
        //Checks if the identifier specifies the relay as normally closed
        if(input.relayList[currentRelay].device[i].find("RESISTOR") != std::string::npos){
            resistorIdentifier = input.relayList[currentRelay].identifier[i];
        }
        //Checks to see if there is a relay on this connection
        for(int j=0; j<input.modelData.size(); j++){
            if(input.relayList[currentRelay].device[i].find(input.modelData[j].relayModel) != std::string::npos){
                presentRelay = true;
                break;
            }
        }
        //Checks if the relay is normally closed and breaks out of the loop
        if(presentRelay && resistorIdentifier != ""){
                //Loops through all the netlist data to find the resistor connections
                for(int j=0; j<input.netlistData.size(); j++){
                    for(int k=0; k<input.netlistData[j].name.size(); k++){
                        //Checks if the current data has a connection to the resistor
                        if(input.netlistData[j].identifier[k] == resistorIdentifier){
                            //Checks if the connection is to ground
                            if(input.netlistData[j].name[k] == "GND"){
                                //If the resistor is connected to ground, the relay is normally closed
                                gndConnection = true;
                                break;
                            }
    
                            //Increments the tracker, will break out of the loop if the resistor has two connections without a ground connection
                            tracker++;
                            if(tracker == 2){
                                break;
                            }
                        }
                    }
    
                    //Breaks out of the loop if the resistor has a ground connection or both connections have been found
                    if(gndConnection || tracker == 2){
                        break;
                    }
                }
    
                //Checks if the resistor is a pulldown resistor
                if(gndConnection){
                    //If the resistor is a pulldown, the relay is normally closed
                    normalState = "RELAY_NC";
                }
            break;
        }
    }

    //Returns the normal state of the relay
    return normalState;
}

void Relay::relayConvert(const Input& input, Relay& relay){
    //Declare and initialize necessary variables
    RelayData temp = RelayData();

    //Calculates and outputs the amount of relays
    relay.relayAmount = input.relayList.size();
    std::cout << "Number of Relays: " << relay.relayAmount << std::endl;

    //Calculates the chain identifier and device modifier based on the board type
    if(convertChain(input) == 1){
        temp.chainNumber = 1;
        relay.deviceModifier = 18;
    }
    else{
        temp.chainNumber = 0;
        relay.deviceModifier = 21;
    }

    //Fills in all relay data
    for(relay.currentRelay=0; relay.currentRelay<relay.relayAmount; relay.currentRelay++){
        //Finds the non dependent relay data and temporarily stores it
        temp.pin = convertPin(input, relay);
        temp.enumeration = convertEnum(input, relay);
        temp.deviceNumber = convertDevice(input, relay);
        temp.portNumber = convertPort(input, relay);
        temp.normalState = convertNormalState(input, relay);
        temp.relayName = input.relayList[relay.currentRelay].name[0];

        //Determines the drive strength of the relay based on the normal state
        if(temp.normalState == "RELAY_NC"){
            temp.driveStrength = "CONFIG_OUT";
        }
        else{
            temp.driveStrength = convertDriveStrength(input, relay);
        }

        //Fills in a line of relay data
        relay.relayData.push_back(temp);
    }

    
    //Returns to main
    return;
}
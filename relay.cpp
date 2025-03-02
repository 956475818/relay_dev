using namespace std;
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct RelayData {
    string pin;
    string enumeration;
    int chainNumber; //This will need to be changed based on user input, Power = 0, Precision = 1
    int deviceNumber;
    string portNumber;
    string driveStrength;
    string normalState;
    int minOpenTime = 0; //This and all the other time values are hard coded in based on what numbers have worked for TI
    int typOpenTime = 60;
    int maxOpenTime = 200;
    int minCloseTime = 0;
    int typCloseTime = 100;
    int maxCloseTime = 500;
    string relayName;
};  //Holds all final relay data

struct NetlistData {
    vector<string> name;
    vector<string> identifier;
    vector<string> connection;
    vector<string> device;
};  //Holds all netlist input data

struct BillData {
    string item;
    string quantity;
    string manufacturer;
    string partNumber;
    vector<string> identifier;
    string description;
    string dni;
    string status;
    string maxTemp;
};  //Holds all BOM input data

struct IOData {
    string identifier;
    int position;
};  //Holds all IO expander data

class Input {
public:
    string netlistName;
    string billName;
	string boardType = "Power";
    vector<BillData> billData;
    vector<IOData> ioData;
    vector<NetlistData> netlistData;
    vector<NetlistData> relayList;
};

void getBillData(Input& input) {
    //This function will be dedicated to putting all of the BOM data into vector<BillData> from the text file converted spreadsheet
    ifstream bill(input.billName);
    string holder;
    BillData temp;
    int loopCounter = 0;

    //Loops as we get information from the BOM line by line
    while (getline(bill, holder)) {
        //Checks if the line is part data, we only want to store part data
        if (isdigit(holder[0])) {
            //Obtains the item number, erases up to the next cell
            temp.item = holder.substr(0, holder.find(","));
            holder.erase(0, holder.find(",") + 1);

            //Obtains the quantity of this part, erases up to the next cell
            temp.quantity = holder.substr(0, holder.find(","));
            holder.erase(0, holder.find(",") + 1);

            //Obtains the manufacturer of the part, erases up to the next cell
            temp.manufacturer = holder.substr(0, holder.find(","));
            holder.erase(0, holder.find(",") + 1);

            //Obtains the part number, erases up to the next cell
            temp.partNumber = holder.substr(0, holder.find(","));
            holder.erase(0, holder.find(",") + 1);

            //Checks if there are multiple identifiers for this part
            if (holder[0] == '\"') {
                //Removes the quotation marks
                holder.erase(0, 1);

                //Runs until the quotation marks end
                while (holder[0] != '\"') {
                    //Checks if the index of the closing quotation mark is larger than the index of the next comma
                    if (holder.find('\"') > holder.find(",")) {
                        //Stores an element into the identifier vector
                        temp.identifier.push_back(holder.substr(0, holder.find(",")));

                        //Erases up to the next identifier
                        holder.erase(0, holder.find(",") + 1);
                    }
                    //Runs if the next comma comes after the quotation mark
                    else {
                        //Stores the last element into the identifier vector
                        temp.identifier.push_back(holder.substr(0, holder.find('\"')));

                        //Erases up to the quotation mark, will break out of the loop
                        holder.erase(0, holder.find('\"'));
                    }
                }

                //Removes the quotation marks and comma
                holder.erase(0, 2);
            }
            //Runs if there is only one identifier
            else {
                //Stores the identifier into the vector
                temp.identifier.push_back(holder.substr(0, holder.find(",")));

                //Erases up to the next cell
                holder.erase(0, holder.find(",") + 1);
            }

            //Obtains the general description of this part, erases up to the next cell
            if (holder[0] == '\"') {
                holder.erase(0, 1);
                temp.description = holder.substr(0, holder.find('\"'));
                holder.erase(0, holder.find('\"') + 2);
            }
            else {
                temp.description = holder.substr(0, holder.find(","));
                holder.erase(0, holder.find(",") + 1);
            }

            //Obtains if the part is not included, erases up to the next cell
            temp.dni = holder.substr(0, holder.find(","));
            holder.erase(0, holder.find(",") + 1);

            //Obtains the part status, erases up to the next cell
            temp.status = holder.substr(0, holder.find(","));
            holder.erase(0, holder.find(",") + 1);

            //Obtains the max temperature of the part, erases the holder for the next cycle
            temp.maxTemp = holder;
            holder.erase();

            //Allocates space for the new bill data vector element
            input.billData.push_back(BillData());

            //Stores all the information from the temporary values into bill data
            input.billData[loopCounter].item = temp.item;
            input.billData[loopCounter].quantity = temp.quantity;
            input.billData[loopCounter].manufacturer = temp.manufacturer;
            input.billData[loopCounter].partNumber = temp.partNumber;
            for (int k = 0; k < temp.identifier.size(); k++) {
                input.billData[loopCounter].identifier.push_back(temp.identifier[k]);
            }
            input.billData[loopCounter].description = temp.description;
            input.billData[loopCounter].dni = temp.dni;
            input.billData[loopCounter].status = temp.status;
            input.billData[loopCounter].maxTemp = temp.maxTemp;

            //Resets the temporary data holders for the next rotation, increments loop counter
            temp.identifier.clear();
            temp = BillData();
            loopCounter++;
        }
    }

    //Return to main
    return;
}

bool checkConnection(NetlistData input) {
    //Declare and initialize necessary variables
    bool numberCheck = true;
    bool connectionCheck = false;
    string connection = input.connection[0];

    //Checks that the connection is a digit
    for (int k = 0; k < connection.size(); k++) {
        //Checks if the character is not a digit
        if (!isdigit(connection[k])) {
            numberCheck = false;
            break;
        }
    }

    //Checks if the connection is a valid connection
    if (numberCheck) {
        //Checks if the connection is a relay based connection
        switch (stoi(connection)) {
        case 11:
        case 20:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
            //Indicates that the information group is not relevant and breaks out of the loop
            break;
        default:
            //Indicates that the information group is relevant and breaks out of the loop
            connectionCheck = true;
            break;
        }
    }

    //Returns the connection check
    return connectionCheck;
}

int getNetlistData(Input& input) {
    //Declare and initialize necessary variables
    ifstream netlist(input.netlistName);
    string holder;
    NetlistData temp;
    string comparator = " ";
    int loopCounter = 0;
    int relayCounter = 0;
    bool relayConnection = false;

    while (getline(netlist, holder)) {
        //Checks if this is the first line, compares to see if on the same relay, adjusts necessary information for proper data storage
        if (comparator != " " && comparator.substr(0, comparator.find(" ")) != holder.substr(0, holder.find(" "))) {
            //Checks for a flag that the previous loop had a relay connection to an IO expander
            if (relayConnection) {
                //Allocates space for the relay input.relayList
                input.relayList.push_back(NetlistData());

                //Allocates and initializes data into the relay input.relayList from the full netlist data
                for (int i = 0; i < input.netlistData[loopCounter].name.size(); i++) {
                    input.relayList[relayCounter].name.push_back(input.netlistData[loopCounter].name[i]);
                    input.relayList[relayCounter].identifier.push_back(input.netlistData[loopCounter].identifier[i]);
                    input.relayList[relayCounter].connection.push_back(input.netlistData[loopCounter].connection[i]);
                    input.relayList[relayCounter].device.push_back(input.netlistData[loopCounter].device[i]);
                }

                //Increments the total amount of relays, allows the program to search for the next IO connection
                relayCounter++;
                relayConnection = false;
            }

            //Increases the loop of the full netlist data
            loopCounter++;
        }

        //Resets the comparator for the next cycle
        comparator.erase();
        comparator = holder;

        //Obtains the first column of information and removes it and the blank spaces to the second column
        temp.name.push_back(holder.substr(0, holder.find(" ")));
        holder.erase(0, holder.find(" "));
        while (holder[0] == ' ') {
            holder.erase(holder.begin());
        }

        //Obtains the second column of information and removes it and the blank spaces to the third column
        temp.identifier.push_back(holder.substr(0, holder.find(" ")));
        holder.erase(0, holder.find(" "));
        while (holder[0] == ' ') {
            holder.erase(holder.begin());
        }

        //Obtains the third column of information and removes it and the blank spaces to the fourth column
        temp.connection.push_back(holder.substr(0, holder.find(" ")));
        holder.erase(0, holder.find(" "));
        while (holder[0] == ' ') {
            holder.erase(holder.begin());
        }

        //Obtains the fourth column of information and erases the holder for the next cycle
        temp.device.push_back(holder);
        holder.erase();

        //Stores the relevant information in netlistData and erases the temporary strings
        input.netlistData.push_back(NetlistData());
        input.netlistData[loopCounter].name.push_back(temp.name[0]);
        input.netlistData[loopCounter].identifier.push_back(temp.identifier[0]);
        input.netlistData[loopCounter].connection.push_back(temp.connection[0]);
        input.netlistData[loopCounter].device.push_back(temp.device[0]);

        //Checks if the current line has a connection to IO expander
        if (temp.device[0] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && !relayConnection) {
            //Checks if the IO expander connection is a relay connection
            if (checkConnection(temp)) {
                //Sets a flag for when the name is finished to store the data in the relay input.relayList
                relayConnection = true;
            }
        }

        //Removes the temporary data to prepare for the next cycle;
        temp.name.clear();
        temp.identifier.clear();
        temp.connection.clear();
        temp.device.clear();
    }

    //Additional final check if the last associated name is a connection to the IO expander
    if (relayConnection) {
        //Allocates space for the relay input.relayList
        input.relayList.push_back(NetlistData());

        //Allocates and initializes data into the relay input.relayList from the full netlist data
        for (int i = 0; i < input.netlistData[loopCounter].name.size(); i++) {
            input.relayList[relayCounter].name.push_back(input.netlistData[loopCounter].name[i]);
            input.relayList[relayCounter].identifier.push_back(input.netlistData[loopCounter].identifier[i]);
            input.relayList[relayCounter].connection.push_back(input.netlistData[loopCounter].connection[i]);
            input.relayList[relayCounter].device.push_back(input.netlistData[loopCounter].device[i]);
        }

        //Increments the total amount of relays, allows the program to search for the next IO connection
        relayCounter++;
        relayConnection = false;
    }

    return relayCounter;
}

void getFirstDevice(vector<NetlistData> netlist, vector<IOData>& list) {
    //Declare and initialize necessary variables
    bool input = false;
    bool output = false;

    //This could be modified to find the last link in the daisy chain as well, not sure how to hold that information or if it will be more efficient
    //Finds the first link in the daisy chain of IO expanders by looping through each piece of netlist data
    for (int i = 0; i < netlist.size(); i++) {
        //Loops through each piece of data associated with the name stored in netlist data
        for (int j = 0; j < netlist[i].name.size(); j++) {
            //Checks if the data has an IO expander connection to DIN
            if (netlist[i].device[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && netlist[i].connection[j] == "33") {
                input = true;
            }
            //Checks if the data has an IO expander connection to DOUT
            else if (netlist[i].device[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && netlist[i].connection[j] == "40") {
                output = true;

                //Exits the loop since we are only searching for the first link, if an output is associated it cannot be the first link
                break;
            }
        }
        //Checks if the data had an IO expander DIN connection but no DOUT connection
        if (input && !output) {
            //Loops through the associated data again to find the position
            for (int k = 0; k < netlist[i].name.size(); k++) {
                //Finds the associated input
                if (netlist[i].device[k] == "MAX6957ATL_TQFN-40-MAX6957ATL+") {
                    //Loops through the identifiers of the IO expanders based on the BOM
                    list[0].identifier = netlist[i].identifier[k];
                    list[0].position = 1;

                    //Breaks out of the search loop for the associated data
                    break;
                }
            }

            //No need to continue searching once the first position has been found
            break;
        }

        //Resets the checks for the next rotation
        input = false;
        output = false;
    }

    //Returns to filter function
    return;
}

void getDeviceOrder(Input& input) {
    //Declare and initialize necessary variables
    bool connectionFound = false;
    bool lastDevice = false;
    int expanders = 0;

    input.ioData.push_back(IOData());

    getFirstDevice(input.netlistData, input.ioData);

    //Loops through all of input.netlistData data until all IO expanders are found
    while (!lastDevice) {
        for (int i = 0; i < input.netlistData.size(); i++) {
            //Loops through each piece of data associated with the name stored in input.netlistData data
            for (int j = 0; j < input.netlistData[i].name.size(); j++) {
                //Checks if the data has an IO expander connection to DOUT
                if (input.netlistData[i].device[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[i].connection[j] == "40" && input.netlistData[i].identifier[j] == input.ioData[expanders].identifier) {
                    //Searches for the corresponding DIN
                    for (int k = 0; k < input.netlistData[i].name.size(); k++) {
                        //If a corresponding DIN is found, the information is obtained
                        if (input.netlistData[i].device[k] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[i].connection[k] == "33") {
                            //Increments the index of the input.ioData of IO expanders, states that the daisy chain continues
                            expanders++;
                            connectionFound = true;

                            //Stores the connection information and breaks the search
                            input.ioData.push_back(IOData());
                            input.ioData[expanders].identifier = input.netlistData[i].identifier[k];
                            input.ioData[expanders].position = expanders + 1;
                            break;
                        }
                    }
                    //If the chain does not continue, break out of the large search loop
                    if (!connectionFound) {
                        lastDevice = true;
                        break;
                    }
                    //If the chain continues, reset the connection for the next IO expander
                    else {
                        connectionFound = false;
                    }
                }
            }
            if (lastDevice) {
                break;
            }
        }
    }

    return;
}

string convertPin(const string& strName, int index) {
    //Declare and initialize necessary variables
    string pin = "DUT_PIN_1";
	int pinNumber;
    char lastChar = strName.back();

    //Finds the pin number based on the last character of the name
    if (isdigit(lastChar)) {
        //Converts the pin number to an integer
        pinNumber = lastChar - '0';

        //Checks if the pin number is a double digit number
        lastChar = strName[strName.size() - 2];
        if (isdigit(lastChar)) {
            //Adjusts the pin number to the correct value
            pinNumber += (lastChar - '0') * 10;
        }

        //Converts the pin number to a string
        pin = "DUT_PIN_" + to_string(pinNumber);
    }

    //Returns the pin number
    return pin;
}

string convertEnum(const string& strName, int index) {
    //Declare and initialize necessary variables
    string enumeration;
    char lastChar = strName.back();
	size_t tracker = -1;
    vector<int> nameBreaks;

    //Finds if the last character of the name is a number
    if (isdigit(lastChar)) {
        //Finds all instances of the underscore in the name
        while ((tracker = strName.find("_", tracker + 1)) != string::npos) {
            nameBreaks.push_back(int(tracker));
        }
        //Replaces the middle information with CIB
        enumeration = strName.substr(0, nameBreaks[0] + 1) + "CIB" + strName.substr(nameBreaks.back(), strName.back() - nameBreaks.back() + 1);
    }
    else {
        //If the end of the name is not a number, the final word is replaced with CIB unless it is a gnd
        if (strName.find("SYSGND") != string::npos) {
            enumeration = strName.substr(0, strName.find("SYSGND")) + "SYSGND";
        }
        else {
            //Finds all instances of the underscore in the name
            while ((tracker = strName.find("_", tracker + 1)) != string::npos) {
                nameBreaks.push_back(int(tracker));
            }
            //Replaces the final word with CIB
            enumeration = strName.substr(0, nameBreaks.back() + 1) + "CIB";
        }
    }

    //Returns the enumeration
    return enumeration;
}

int converChain(const string& boardType) {
    //Declare and initialize necessary variables
	int chainNumber = 0;

    //Determines the chain number based on the board type
    if (boardType == "Power")
        chainNumber = 0;
    else if (boardType == "Precision")
        chainNumber = 1;

    //Returns the chain number
    return chainNumber;
}

int convertDevice(const vector<NetlistData>& input, int index, string boardType, const vector<IOData>& expanders) {
    //Declare and initialize necessary variables
    int deviceNumber;

    //Determines which line of the netlist data has the device number
    for (int i = 0; i < input[index].device.size(); i++) {
        //Find the index of the connection to the IO expander
        if (input[index].device[i] == "MAX6957ATL_TQFN-40-MAX6957ATL+") {
            //Loops through each IO expander
            for (int j = 0; j < expanders.size(); j++) {
                //Checks which IO expander is associated with this relay
                if (expanders[j].identifier == input[index].identifier[i]) {
                    deviceNumber = expanders[j].position;
                    break;
                }
            }
            break;
        }
    }

    //Adjusts the device number based on the board type
    if (boardType == "Power")
        deviceNumber += 21;
    else if (boardType == "Precision") {
        deviceNumber += 18;
    }

    //Returns the device number
    return deviceNumber;
}

string convertPort(const vector<NetlistData>& input, int index) {
    //Declare and initialize necessary variables
    string portIdentifier;
    int connectionNumber = 0;
    string portNumber;

    //Determines which line of the netlist data has the port connection
    for (int i = 0; i < input[index].device.size(); i++) {
        //Find the index of the connection to the IO expander
        if (input[index].device[i] == "MAX6957ATL_TQFN-40-MAX6957ATL+") {
            //Assigns the port identifier based on connection to the IO expander
            portIdentifier = input[index].connection[i];
            break;
        }
    }

    //Converts the port identifier to the corresponding connection number
    connectionNumber = stoi(portIdentifier);

    //Determines the port number based on the connection number and returns it
    switch (connectionNumber) {
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

string convertDriveStrength(const vector<NetlistData>& input, int index) {
    //Declare and initialize necessary variable, relays are set to 10.5 mA by default
    string driveStrength = "CONFIG_LED_10p5_mA";

    //Checks each identifier for the relay for non default cases
    for (int i = 0; i < input[index].identifier.size(); i++) {
        //Checks if the identifier specifies the relay as normally closed
        if (input[index].identifier[i].find("_NC_") != string::npos) {
            driveStrength = "CONFIG_OUT";
            break;
        }
        //Checks if the name specifies the relay as an enable
        else if (input[index].name[i].find("_EN") != string::npos) {
            driveStrength = "CONFIG_OUT";
            break;
        }
        else
		{        
            //Checks if the relay type will change the drive strength from the default
            /*
				Coto 9901-05-20 --- 18.0mA
				Omron G3VM-101QR1 --- 10.5mA
				Omron G3VM-41QR10 --- 10.5mA
				Panasonic AQY225R3TY --- 10.5mA
				Pickering 120-1-A-5/1 --- 21.0mA
				Tyco 50935 --- 21.0mA
			*/
			string device = input[index].device[i];
			if (device.find("9901-05-20") != string::npos)
			{
				driveStrength = "CONFIG_LED_18p0_mA";
				break;
			}
			else if (device.find("G3VM-101QR1") != string::npos || device.find("G3VM-41QR10") != string::npos || device.find("AQY225R3TY") != string::npos)
			{
				driveStrength = "CONFIG_LED_10p5_mA";
                break;
			}
			else if (device.find("120-1-A-5/1") != string::npos || device.find("50935") != string::npos)
			{
				driveStrength = "CONFIG_LED_21p0_mA";
				break;
			}
		}
    }

    return driveStrength;
}

string convertNormalState(const vector<NetlistData>& input, int index) {
    //Declare and initialize necessary variables, relays are normally open by default
    string normalState = "RELAY_NO";

    //Checks if any of the identifiers specify the relay is normally closed
    for (int i = 0; i < input[index].identifier.size(); i++) {
        //Checks if the identifier specifies the relay as normally closed
        if (input[index].identifier[i].find("_NC_") != string::npos) {
            normalState = "RELAY_NC";
            break;
        }
    }

    //Returns the normal state of the relay
    return normalState;
}

void relayConvert(const Input& input, int relayAmount, vector<RelayData>& relay) {
    //Declare and initialize necessary variables
    RelayData temp;

    //Fills in all relay data
    for (int i = 0; i < relayAmount; i++) {
        //Finds the interpreter based relay data and temporarily stores it
		string strName = input.relayList[i].name[0];
        temp.pin = convertPin(strName, i);
        temp.enumeration = convertEnum(strName, i);
        temp.chainNumber = converChain(input.boardType);
        temp.deviceNumber = convertDevice(input.relayList, i, input.boardType, input.ioData);
        temp.portNumber = convertPort(input.relayList, i);
        temp.driveStrength = convertDriveStrength(input.relayList, i);
        temp.normalState = convertNormalState(input.relayList, i);
        temp.relayName = strName;

        //Fills in a line of relay data
        relay.push_back(temp);
    }
}

void writeCSV(const vector<RelayData>& relay, string outputName, int relayAmount) {
    //Declare and initialize necessary variables
    ofstream file(outputName);

    //Checks if the file was opened successfully
    if (!file.is_open()) {
		cerr << "Failed to open file " << outputName << "\n";
        return;
    }

    //Write to the file line by line the relay data in CSV format
    for (int i = 0; i < relayAmount; i++) {
        file<<"RELAY(" << relay[i].pin + "," + relay[i].enumeration + "," + to_string(relay[i].chainNumber) + "," +
            to_string(relay[i].deviceNumber) + "," + relay[i].portNumber + "," + relay[i].driveStrength + "," +
            relay[i].normalState + "," + to_string(relay[i].minOpenTime) + "," + to_string(relay[i].typOpenTime) +
            "," + to_string(relay[i].maxOpenTime) + "," + to_string(relay[i].minCloseTime) + "," + to_string(relay[i].typCloseTime) +
			"," + to_string(relay[i].maxCloseTime) + "," + relay[i].relayName + ")";

        if (i < relayAmount - 1) {
            file << "\n";
        }
    }

    //Close the file and output that the file was created successfully
    file.close();
	cout << outputName << " export successfully!\n";
}


int main(int argc, char* argv[]) {
	string strBom, strNetlist;
	string strOutput = "result.txt";
	if (argc >= 3)
	{
		strBom = argv[1];
        strNetlist = argv[2];
        if (argc == 4)
			strOutput = argv[3];
	}
    else
    {
#ifdef _DEBUG
        strBom = "D:\\Workspace\\dev\\BOM.txt";
        strNetlist = "D:\\Workspace\\dev\\netlist.txt";
#endif // _DEBUG
    }

    if(strBom.empty() || strNetlist.empty())
	{
		cout << "Please provide the BOM and Netlist file paths\n";
		return 1;
	}

    //Declare and initialize necessary variables
    vector<RelayData> relayData;
    int relayAmount;
    Input input;
    input.billName = strBom;
    input.netlistName = strNetlist;

    //Outputs the names of the netlist and BOM files
    cout << input.netlistName + "\n";
    cout << input.billName + "\n";

    //The function to obtain the BOM data will be added here
    getBillData(input);

    //Parses through netlist and puts data into data structure, obtains and outputs amount of relays
    relayAmount = getNetlistData(input);
    cout << to_string(relayAmount) + "\n";

    //The function to get the order of the IO expanders
    getDeviceOrder(input);

    //Converts information from netlistData to relayData
	relayConvert(input, relayAmount, relayData);

    //Outputs the relay data in CSV format
    writeCSV(relayData, strOutput, relayAmount);

    return 0;
}

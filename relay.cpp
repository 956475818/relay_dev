#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

struct RelayData {
	std::string pin;            //����
	std::string enumeration;    //ö��
	int chainNumber;            //This will need to be changed based on user input, Power = 0, Precision = 1
	int deviceNumber;
	std::string portNumber;
	std::string driveStrength;
	std::string normalState;
	int minOpenTime = 0; //This and all the other time values are hard coded in based on what numbers have worked for TI
	int typOpenTime = 60;
	int maxOpenTime = 200;
	int minCloseTime = 0;
	int typCloseTime = 100;
	int maxCloseTime = 500;
	std::string relayName;
};  //Holds all final relay data

struct NetlistData {
	std::vector<std::string> vecName;           //����    K10_1800PF_1
	std::vector<std::string> vecIdentifier;     //��ʶ��  K10_1
	std::vector<std::string> vecConnection;     //����    2
	std::vector<std::string> vecDevice; 	    //�豸    MAX6957ATL_TQFN-40-MAX6957ATL+
};  //Holds all netlist input data

struct BillData {
	std::string item;
	std::string quantity;
	std::string manufacturer;
	std::string partNumber;
	std::vector<std::string> identifier;
	std::string description;
	std::string dni;
	std::string status;
	std::string maxTemp;
};  //Holds all BOM input data

struct IOData {
	std::string identifier;
	int position;
};  //Holds all IO expander data

/*
struct ModelData{
  string relayModel;
  vector<double> switchTimes;
  string normalState;
};  //Holds all information directly tied to the relay model
Both of the above structures will not be used in the proof of concept
*/

class Input {
private:
	/*
	string getName(string);
	string getIdentifier(string);
	string getConnection(string);
	string getDevice(string);
	*/
	bool checkConnection(NetlistData);
	void getFirstDevice(std::vector<NetlistData>, std::vector<IOData>&);
	// string getPartName(string);
	// vector<string> getAssociatedDevices(string); 
	// string getPartType(string);
public:
	std::string netlistPath;
	std::string bomPath;
	std::string boardType;
	std::vector<BillData> billData;
	std::vector<IOData> ioData;
	std::vector<NetlistData> netlistData;
	std::vector<NetlistData> relayList;
	/*
	* �̵�������ǿ�� ֻ�����ֶΣ�����Ҫ�ṹ�壬ֱ����map�������
		Coto 9901-05-20 --- 18.0mA
		Omron G3VM-101QR1 --- 10.5mA
		Omron G3VM-41QR10 --- 10.5mA
		Panasonic AQY225R3TY --- 10.5mA
		Pickering 120-1-A-5/1 --- 21.0mA
		Tyco 50935 --- 21.0mA
	*/
	std::map<std::string, std::string> mapDriveStrength = {
		{"9901-05-20", "CONFIG_LED_18p0_mA"},
		{"G3VM-101QR1", "CONFIG_LED_10p5_mA"},
		{"G3VM-41QR10", "CONFIG_LED_10p5_mA"},
		{"AQY225R3TY", "CONFIG_LED_10p5_mA"},
		{"120-1-A-5/1", "CONFIG_LED_21p0_mA"},
		{"50935", "CONFIG_LED_21p0_mA"}
	};

	////��������GND������ ��ʶ�����豸����   K35_TMU_CNTL             K35_NC_TMU       2        AQY22XRXT_SMT-4P-AQY225R3TY
	std::vector<std::string> gndData;   //K35_TMU_CNTL_K35_NC_TMU_AQY22XRXT_SMT-4P-AQY225R3TY

	int getNetlistData(Input&);
	int filterNetlistData(std::vector<NetlistData>, std::vector<NetlistData>&);
	void getBillData(Input&);
	void getDeviceOrder(Input&);
};

class Relay {
private:
	std::string convertPin(std::string);
	std::string convertEnum(std::string);
	int convertChain(std::string);
	int convertDevice(std::vector<NetlistData>, int, std::string, std::vector<IOData>);
	std::string convertPort(std::vector<NetlistData>, int);
	std::string convertDriveStrength(std::vector<NetlistData>, int);
	std::string convertNormalState(std::vector<NetlistData>, int);
	//string convertName(vector<NetlistData>, int);

	// The below three functions will need to be adjusted to obtain the drive strength and normal state
	// string getDriveStrength(file_data);
	// void getModelData(file_data, &model_data);
	// void convertModelData(&relay_data, model_data);
public:
	std::vector<RelayData> relayData;
	int relayAmount;
	// vector<model_Data> modelData;
	void relayConvert(Input&, Relay&, int);
	void relayAnalyze(std::vector<NetlistData>, std::vector<RelayData>&, int);
};

class Output {
public:
	std::string outputName;
	void writeIntermediateData(std::vector<RelayData>, std::vector<NetlistData>, std::string, int);
	void writeCSV(Relay&, std::string);
	void writeJSON(std::vector<RelayData>&, std::string, int);
};

void getBillData(Input& input) {
	//���ж�ȡBOM�ļ��������ݴ洢��billData��
	std::ifstream bill(input.bomPath);
	std::string holder;
	BillData temp;
	int loopCounter = 0;

	//Loops as we get information from the BOM line by line
	while (getline(bill, holder)) {
		//�жϸ��е�һ���Ƿ�Ϊ����
		//���°����洢���ݵ�billData��
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
}

bool checkConnection(NetlistData input) {
	//Declare and initialize necessary variables
	bool numberCheck = true;
	bool connectionCheck = false;
	std::string connection = input.vecConnection[0];

	//Checks that the connection is a digit
	for (int k = 0; k < connection.size(); k++) {
		//Checks if the character is not a digit
		//�ж��Ƿ�Ϊ����
		if (!isdigit(connection[k])) {
			numberCheck = false;
			break;
		}
	}

	//�ж��Ƿ�����Ч������
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
	std::ifstream netlist(input.netlistPath);
	std::string holder;
	NetlistData temp;
	std::string comparator = " ";
	int loopCounter = 0;
	int relayCounter = 0;
	bool relayConnection = false;

	//���ж�ȡnetlist�ļ��������ݴ洢��netlistData��
	while (getline(netlist, holder)) {
		//Checks if this is the first line, compares to see if on the same relay, adjusts necessary information for proper data storage
		if (comparator != " " && comparator.substr(0, comparator.find(" ")) != holder.substr(0, holder.find(" "))) {
			//Checks for a flag that the previous loop had a relay connection to an IO expander
			//�����һ��ѭ���м̵������ӵ�IO��չ���������ݴ洢��relayList��
			if (relayConnection) {
				//Allocates space for the relay list
				input.relayList.push_back(NetlistData());

				//Allocates and initializes data into the relay list from the full netlist data
				for (int i = 0; i < input.netlistData[loopCounter].vecName.size(); i++) {
					input.relayList[relayCounter].vecName.push_back(input.netlistData[loopCounter].vecName[i]);
					input.relayList[relayCounter].vecIdentifier.push_back(input.netlistData[loopCounter].vecIdentifier[i]);
					input.relayList[relayCounter].vecConnection.push_back(input.netlistData[loopCounter].vecConnection[i]);
					input.relayList[relayCounter].vecDevice.push_back(input.netlistData[loopCounter].vecDevice[i]);
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
		temp.vecName.push_back(holder.substr(0, holder.find(" ")));
		holder.erase(0, holder.find(" "));
		while (holder[0] == ' ') {
			holder.erase(holder.begin());
		}

		//Obtains the second column of information and removes it and the blank spaces to the third column
		temp.vecIdentifier.push_back(holder.substr(0, holder.find(" ")));
		holder.erase(0, holder.find(" "));
		while (holder[0] == ' ') {
			holder.erase(holder.begin());
		}

		//Obtains the third column of information and removes it and the blank spaces to the fourth column
		temp.vecConnection.push_back(holder.substr(0, holder.find(" ")));
		holder.erase(0, holder.find(" "));
		while (holder[0] == ' ') {
			holder.erase(holder.begin());
		}

		//Obtains the fourth column of information and erases the holder for the next cycle
		temp.vecDevice.push_back(holder);
		holder.erase();

		//Checks if the current line has a connection to IO expander
		if (temp.vecDevice[0] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && !relayConnection) {
			//Checks if the IO expander connection is a relay connection
			//�������������ݴ洢��relayList��
			if (checkConnection(temp)) {
				//Sets a flag for when the name is finished to store the data in the relay list
				relayConnection = true;
			}
		}

		//Stores the relevant information in netlistData
		input.netlistData.push_back(NetlistData());
		std::string strName = temp.vecName[0];
		std::string strIdentifier = temp.vecIdentifier[0];
		std::string strDevice = temp.vecDevice[0];

		input.netlistData[loopCounter].vecName.push_back(strName);
		input.netlistData[loopCounter].vecIdentifier.push_back(strIdentifier);
		input.netlistData[loopCounter].vecConnection.push_back(temp.vecConnection[0]);
		input.netlistData[loopCounter].vecDevice.push_back(strDevice);

		//����GND������
		if (strName == "GND" && strIdentifier[0] == 'K')
			input.gndData.push_back(strIdentifier + "_" + strDevice);

		//Removes the temporary data to prepare for the next cycle;
		temp.vecName.clear();
		temp.vecIdentifier.clear();
		temp.vecConnection.clear();
		temp.vecDevice.clear();
	}

	//Additional final check if the last associated name is a connection to the IO expander
	//���ֱ�ʶ�������ݴ洢��relayList��
	if (relayConnection) {
		//Allocates space for the relay list
		input.relayList.push_back(NetlistData());

		//Allocates and initializes data into the relay list from the full netlist data
		for (int i = 0; i < input.netlistData[loopCounter].vecName.size(); i++) {
			input.relayList[relayCounter].vecName.push_back(input.netlistData[loopCounter].vecName[i]);
			input.relayList[relayCounter].vecIdentifier.push_back(input.netlistData[loopCounter].vecIdentifier[i]);
			input.relayList[relayCounter].vecConnection.push_back(input.netlistData[loopCounter].vecConnection[i]);
			input.relayList[relayCounter].vecDevice.push_back(input.netlistData[loopCounter].vecDevice[i]);
		}

		//Increments the total amount of relays, allows the program to search for the next IO connection
		relayCounter++;
		relayConnection = false;
	}

	return relayCounter;
}

void getFirstDevice(std::vector<NetlistData> netlist, std::vector<IOData>& list) {
	//Declare and initialize necessary variables
	bool input = false;
	bool output = false;

	//This could be modified to find the last link in the daisy chain as well, not sure how to hold that information or if it will be more efficient
	//Finds the first link in the daisy chain of IO expanders by looping through each piece of netlist data
	//ѭ������netlist���ݵ�ÿһ���֣��ҵ�IO��չ���ĵ�һ������
	for (int i = 0; i < netlist.size(); i++) {
		//Loops through each piece of data associated with the name stored in netlist data
		//ѭ��������netlist�����д洢�����ƹ�����ÿ������
		for (int j = 0; j < netlist[i].vecName.size(); j++) {
			//Checks if the data has an IO expander connection to DIN
			//��������Ƿ����IO��չ�����ӵ�DIN
			if (netlist[i].vecDevice[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && netlist[i].vecConnection[j] == "33") {
				input = true;
			}
			//Checks if the data has an IO expander connection to DOUT
			//��������Ƿ����IO��չ�����ӵ�DOUT
			else if (netlist[i].vecDevice[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && netlist[i].vecConnection[j] == "40") {
				output = true;

				//Exits the loop since we are only searching for the first link, if an output is associated it cannot be the first link
				break;
			}
		}
		//Checks if the data had an IO expander DIN connection but no DOUT connection
		//�ж������Ƿ����IO��չ��DIN���ӣ���û��DOUT����
		if (input && !output) {
			//�ٴ�ѭ�����������������ҵ�λ��
			for (int k = 0; k < netlist[i].vecName.size(); k++) {
				//Finds the associated input
				if (netlist[i].vecDevice[k] == "MAX6957ATL_TQFN-40-MAX6957ATL+") {
					//����BOMѭ������IO��չ���ı�ʶ��
					list[0].identifier = netlist[i].vecIdentifier[k];
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
}

void getDeviceOrder(Input& input) {
	//Declare and initialize necessary variables
	bool connectionFound = false;
	bool lastDevice = false;
	int expanders = 0;

	input.ioData.push_back(IOData());

	getFirstDevice(input.netlistData, input.ioData);

	//ѭ����������netlist���ݣ�ֱ���ҵ�����IO��չ��
	while (!lastDevice) {
		for (int i = 0; i < input.netlistData.size(); i++) {
			//ѭ��������netlist�����д洢�����ƹ�����ÿ������
			for (int j = 0; j < input.netlistData[i].vecName.size(); j++) {
				//��������Ƿ����IO��չ�����ӵ�DOUT
				if (input.netlistData[i].vecDevice[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[i].vecConnection[j] == "40" && input.netlistData[i].vecIdentifier[j] == input.ioData[expanders].identifier) {
					//Searches for the corresponding DIN
					//������Ӧ��DIN
					for (int k = 0; k < input.netlistData[i].vecName.size(); k++) {
						//����ҵ���Ӧ��DIN�����ȡ��Ϣ
						if (input.netlistData[i].vecDevice[k] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[i].vecConnection[k] == "33") {
							//����IO��չ���б������
							expanders++;
							connectionFound = true;

							//����������Ϣ
							input.ioData.push_back(IOData());
							input.ioData[expanders].identifier = input.netlistData[i].vecIdentifier[k];
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
}

std::string convertPin(std::string relayName) {
	//Declare and initialize necessary variables
	std::string pin;
	int pinNumber;
	char lastChar = relayName.back();

	//�������Ƶ����һ���ַ��ҵ����ű��
	if (isdigit(lastChar)) {
		//Converts the pin number to an integer
		pinNumber = lastChar - '0';

		//Checks if the pin number is a double digit number
		lastChar = relayName[relayName.size() - 2];
		if (isdigit(lastChar)) {
			//Adjusts the pin number to the correct value
			pinNumber += (lastChar - '0') * 10;
		}

		//Converts the pin number to a string
		pin = "DUT_PIN_" + std::to_string(pinNumber);
	}
	else {
		//������Ƶ�ĩβ�������֣���������ΪĬ������
		pin = "DUT_PIN_1";
	}

	//�������ű��
	return pin;
}

std::string convertEnum(std::string relayName) {
	//Declare and initialize necessary variables
	std::string enumeration;
	char lastChar = relayName.back();
	int tracker = -1;
	std::vector<int> nameBreaks;

	//�������Ƶ����һ���ַ��ҵ�ö��
	if (isdigit(lastChar)) {
		//�ҵ������������»��ߵ�ʵ��
		while ((tracker = relayName.find("_", tracker + 1)) != std::string::npos) {
			nameBreaks.push_back(tracker);
		}
		//���м���Ϣ�滻ΪCIB
		enumeration = relayName.substr(0, nameBreaks[0] + 1) + "CIB" + relayName.substr(nameBreaks.back(), relayName.back() - nameBreaks.back() + 1);
	}
	else {
		//������Ƶ�ĩβ�������֣���������ΪĬ������
		if (relayName.find("SYSGND") != std::string::npos) {
			enumeration = relayName.substr(0, relayName.find("SYSGND")) + "SYSGND";
		}
		else {
			//�ҵ������������»��ߵ�ʵ��
			while ((tracker = relayName.find("_", tracker + 1)) != std::string::npos) {
				nameBreaks.push_back(tracker);
			}
			//���м���Ϣ�滻ΪCIB
			enumeration = relayName.substr(0, nameBreaks.back() + 1) + "CIB";
		}

	}

	//����ö��
	return enumeration;
}

int converChain(std::string boardType) {
	//Declare and initialize necessary variables
	int chainNumber = 0;

	//��������ȷ�������
	if (boardType == "Power")
		chainNumber = 0;
	else if (boardType == "Precision")
		chainNumber = 1;

	//���������
	return chainNumber;
}

int convertDevice(std::vector<NetlistData> input, int index, std::string boardType, std::vector<IOData> expanders) {
	//Declare and initialize necessary variables
	int deviceNumber = 0;

	//�ҵ��豸��ŵ�netlist������
	for (int i = 0; i < input[index].vecDevice.size(); i++) {
		//�ҵ���IO��չ�������ӵ�����
		if (input[index].vecDevice[i] == "MAX6957ATL_TQFN-40-MAX6957ATL+") {
			//ѭ������ÿ��IO��չ��
			for (int j = 0; j < expanders.size(); j++) {
				//����IO��չ��ȷ���豸���
				if (expanders[j].identifier == input[index].vecIdentifier[i]) {
					deviceNumber = expanders[j].position;
					break;
				}
			}
			break;
		}
	}

	//�������͵����豸���
	if (boardType == "Power")
		deviceNumber += 21;
	else if (boardType == "Precision") {
		deviceNumber += 18;
	}

	//�����豸���
	return deviceNumber;
}

std::string convertPort(std::vector<NetlistData> input, int index) {
	//Declare and initialize necessary variables
	std::string portIdentifier;
	int connectionNumber = 0;
	std::string portNumber;

	//�ҵ���IO��չ�������ӵ�netlist������
	for (int i = 0; i < input[index].vecDevice.size(); i++) {
		//�ҵ���IO��չ�������ӵ�����
		if (input[index].vecDevice[i] == "MAX6957ATL_TQFN-40-MAX6957ATL+") {
			//������IO��չ�������ӷ���˿ڱ�ʶ��
			portIdentifier = input[index].vecConnection[i];
			break;
		}
	}

	//���˿ڱ�ʶ��ת��Ϊ��Ӧ�����ӱ��
	connectionNumber = stoi(portIdentifier);

	//�������ӱ��ȷ���˿ڱ�Ų�����
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

std::string getStrengthFromMap(std::string device, const Input& input) {
	std::string strength;
	//����mapDriveStrength ����strength
	for (auto it = input.mapDriveStrength.begin(); it != input.mapDriveStrength.end(); it++) {
		if (device.find(it->first) != std::string::npos) {
			strength = it->second;
			break;
		}
	}

	return strength;
}

std::string convertDriveStrength(const Input& input, int index) {
	std::vector<NetlistData> netlist = input.relayList;
	//Declare and initialize necessary variable, relays are set to 10.5 mA by default
	std::string driveStrength = "CONFIG_LED_10p5_mA";
	std::string device;

	//����ÿ����ʶ���Բ��ҷ�Ĭ�����
	for (int i = 0; i < netlist[index].vecIdentifier.size(); i++) {
		//����ʶ���Ƿ�ָ���̵���Ϊ_NC
		if (netlist[index].vecIdentifier[i].find("_NC_") != std::string::npos) {
			driveStrength = "CONFIG_OUT";
			break;
		}
		//��������Ƿ�ָ���̵���Ϊ_EN
		else if (netlist[index].vecName[i].find("_EN") != std::string::npos) {
			driveStrength = "CONFIG_OUT";
			break;
		}
		else {
			//�̵�������ǿ�Ȼ�ȡ
			driveStrength = getStrengthFromMap(netlist[index].vecDevice[i], input);
			if (!driveStrength.empty())
				break;
		}
	}

	return driveStrength;
}

std::string convertNormalState(const Input& input, int index) {
	std::vector<NetlistData> netdisk = input.relayList;
	//Declare and initialize necessary variables, relays are normally open by default
	std::string normalState = "RELAY_NO";

	//����
	for (int i = 0; i < netdisk[index].vecIdentifier.size(); i++) {
		//ͨ����ʶ��+_+�豸����ƥ���Ƿ�������GND
		std::string strId = netdisk[index].vecIdentifier[i];
		//�жϵ�һ���ַ��Ƿ�Ϊ"K"
		if (strId[0] != 'K')
			continue;

		std::string str = strId + "_" + netdisk[index].vecDevice[i];
		for (auto& name : input.gndData)
		{
			if (name == str)
			{
				normalState = "RELAY_NC";
				break;
			}
		}

		//         //����ʶ���Ƿ�ָ���̵���Ϊ_NC
		//         if(input[index].vecIdentifier[i].find("_NC_") != std::string::npos){
		//             normalState = "RELAY_NC";
		//             break;
		//         }
	}

	//Returns the normal state of the relay
	return normalState;
}

void relayConvert(Input& input, Relay& relay) {
	//Declare and initialize necessary variables
	RelayData temp;
	std::string relayName;
	temp = RelayData();

	//Fills in all relay data
	for (int i = 0; i < relay.relayAmount; i++) {
		//���ݼ̵��������ҵ�����������ʱ�洢
		relayName = input.relayList[i].vecName[0];
		temp.pin = convertPin(relayName);                   //���ݼ̵��������ҵ����ű��
		temp.enumeration = convertEnum(relayName);          //���ݼ̵��������ҵ�ö��
		temp.chainNumber = converChain(input.boardType);    //���ݰ������ҵ������
		temp.deviceNumber = convertDevice(input.relayList, i, input.boardType, input.ioData);   //����IO��չ���ҵ��豸���
		temp.portNumber = convertPort(input.relayList, i);  //����IO��չ���ҵ��˿ڱ��
		temp.driveStrength = convertDriveStrength(input, i);  //���ݼ̵��������ҵ�����ǿ��
		temp.normalState = convertNormalState(input, i);  //���ݼ̵��������ҵ�״̬
		temp.relayName = relayName;						 //�洢�̵�������

		//Fills in a line of relay data
		relay.relayData.push_back(temp);
	}


	//Returns to main
	return;
}

void writeCSV(Relay& relay, std::string outputName) {
	//Declare and initialize necessary variables
	std::ofstream file(outputName);

	//�ļ���׳�Լ��
	if (!file.is_open()) {
		std::cerr << "Failed to open file!\n";
		return;
	}

	//���н��̵���������CSVָ����ʽд���ļ�
	for (int i = 0; i < relay.relayAmount; i++) {
		file << relay.relayData[i].pin + "," + relay.relayData[i].enumeration + "," + std::to_string(relay.relayData[i].chainNumber) + "," +
			std::to_string(relay.relayData[i].deviceNumber) + "," + relay.relayData[i].portNumber + "," + relay.relayData[i].driveStrength + "," +
			relay.relayData[i].normalState + "," + std::to_string(relay.relayData[i].minOpenTime) + "," + std::to_string(relay.relayData[i].typOpenTime) +
			"," + std::to_string(relay.relayData[i].maxOpenTime) + "," + std::to_string(relay.relayData[i].minCloseTime) + "," + std::to_string(relay.relayData[i].typCloseTime) +
			"," + std::to_string(relay.relayData[i].maxCloseTime) + "," + relay.relayData[i].relayName;

		if (i < relay.relayAmount - 1) {
			file << "\n";
		}
	}

	//�ر��ļ�������ļ������ɹ�
	file.close();
	std::cout << "CSV file created successfully\n";
}

int main(int argc, char* argv[]) {
	//�ⲿ���ô����������һ������ΪBOM�ļ�·�����ڶ�������Ϊnetlist�ļ�·��������������Ϊ����ļ�·��
	//�봫������������������������ѡ��Ĭ��Ϊ��ǰĿ¼��Configuration.csv
	//eg: relay.exe D:/Workspace/dev/BOM.txt D:/Workspace/dev/netlist.txt D:/Workspace/dev/Configuration.csv
	std::string strBom, strNetlist;
	Output output;
	output.outputName = "Configuration.csv";
	if (argc >= 3)
	{
		strBom = argv[1];
		strNetlist = argv[2];
		if (argc == 4)
			output.outputName = argv[3];
	}
	else
	{
#ifdef _DEBUG
		strBom = "D:\\Workspace\\dev\\BOM.txt";
		strNetlist = "D:\\Workspace\\dev\\netlist.txt";
#endif // _DEBUG
	}

	if (strBom.empty() || strNetlist.empty())
	{
		std::cout << "Please provide the BOM and Netlist file paths\n";
		return 1;
	}

	//��ʼ������������ݽṹ
	Input input;
	Relay relay;
	input.bomPath = strBom;
	input.netlistPath = strNetlist;
	input.boardType = "Power";

	//��ӡnetlist��BOM�ļ�·��
	std::cout << input.netlistPath + "\n";
	std::cout << input.bomPath + "\n";

	//��ȡBOM����
	getBillData(input);

	//ͨ��netlist�������ݲ������ݷ������ݽṹ�У����ؼ̵�������
	relay.relayAmount = getNetlistData(input);
	std::cout << std::to_string(relay.relayAmount) + "\n";

	//The function to get the order of the IO expanders
	//��ȡIO��չ��
	getDeviceOrder(input);

	//�洢���м̵�������
	relayConvert(input, relay);

	//�����̵������ݵ�CSV�ļ����ļ���ʽ�ɸģ�
	writeCSV(relay, output.outputName);

	return 0;
}
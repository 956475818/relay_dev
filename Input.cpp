#include "Input.h"

void Input::getBillData(Input& input) {
	//This function will be dedicated to putting all of the BOM data into vector<BillData> from the text file converted spreadsheet
	std::ifstream bill(input.billName);
	std::string holder;
	BillData temp;
	int loopCounter = 0;

	//Outputs the name of the BOM file
	std::cout << "Bill of Materials File Path: " << input.billName << std::endl;

	//Loops as we get information from the BOM line by line
	while(getline(bill, holder)){
		holder = toUpper(holder);
	    //Checks if the line is part data, we only want to store part data
	    if(isdigit(holder[0])){
	        //Obtains the item number, erases up to the next cell
	        temp.item = holder.substr(0, holder.find(","));
	        holder.erase(0, holder.find(",")+1);
	        
	        //Obtains the quantity of this part, erases up to the next cell
	        temp.quantity = holder.substr(0, holder.find(","));
	        holder.erase(0, holder.find(",")+1);

	        //Obtains the manufacturer of the part, erases up to the next cell
	        temp.manufacturer = holder.substr(0, holder.find(","));
	        holder.erase(0, holder.find(",")+1);

	        //Obtains the part number, erases up to the next cell
	        temp.partNumber = holder.substr(0, holder.find(","));
	        holder.erase(0, holder.find(",")+1);

	        //Checks if there are multiple identifiers for this part
	        if(holder[0] == '\"'){
	            //Removes the quotation marks
	            holder.erase(0,1);
	            
	            //Runs until the quotation marks end
	            while(holder[0] != '\"'){
	                //Checks if the index of the closing quotation mark is larger than the index of the next comma
	                if(holder.find('\"')>holder.find(",")){
	                    //Stores an element into the identifier vector
	                    temp.identifier.push_back(holder.substr(0, holder.find(",")));

	                    //Erases up to the next identifier
	                    holder.erase(0, holder.find(",")+1);
	                }
	                //Runs if the next comma comes after the quotation mark
	                else{
	                    //Stores the last element into the identifier vector
	                    temp.identifier.push_back(holder.substr(0, holder.find('\"')));

	                    //Erases up to the quotation mark, will break out of the loop
	                    holder.erase(0, holder.find('\"'));
	                }
	            }

	            //Removes the quotation marks and comma
	            holder.erase(0,2);
	        }
	        //Runs if there is only one identifier
	        else{
	            //Stores the identifier into the vector
	            temp.identifier.push_back(holder.substr(0, holder.find(",")));

	            //Erases up to the next cell
	            holder.erase(0, holder.find(",")+1);
	        }

	        //Obtains the general description of this part, erases up to the next cell
	        if(holder[0] == '\"'){
	            holder.erase(0,1);
	            temp.description = holder.substr(0, holder.find('\"'));
	            holder.erase(0, holder.find('\"')+2);
	        }
	        else{
	            temp.description = holder.substr(0, holder.find(","));
	            holder.erase(0, holder.find(",")+1);
	        }

	        //Obtains if the part is not included, erases up to the next cell
	        temp.dni = holder.substr(0, holder.find(","));
	        holder.erase(0, holder.find(",")+1);

	        //Obtains the part status, erases up to the next cell
	        temp.status = holder.substr(0, holder.find(","));
	        holder.erase(0, holder.find(",")+1);

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
	        for(int k=0; k<temp.identifier.size(); k++){
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

	////Return to main
	//return;

	//// 改成从数据库中获取数据
	//try {
	//	auto table = conn->getSchema("relay").getTable("inputbom");
	//	auto result = table.select("CIB_ID", "ITEM", "QTY", "MFG", "MFG_PART_NUM", "REF_DES", "DESCRIPTION", "DNI", "STATUS", "MAX_TEMP")
	//		.execute();
	//	for (const auto& row : result) {
	//		input.billData.push_back(BillData());
	//		input.billData.back().item = row[1].get<int>();
	//		input.billData.back().quantity = row[2].get<int>();
	//		input.billData.back().manufacturer = row[3].get<std::string>();
	//		input.billData.back().partNumber = row[4].get<std::string>();
	//		input.billData.back().identifier.push_back(row[5].get<std::string>());
	//		input.billData.back().description = row[6].get<std::string>();
	//		input.billData.back().dni = row[7].get<std::string>();
	//		input.billData.back().status = row[8].get<std::string>();
	//		input.billData.back().maxTemp = row[9].get<std::string>();
	//	}
	//}
	//catch (const drogon::orm::DrogonDbException& e) {
	//	std::cerr << "Database error: " << e.base().what() << std::endl;
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Error: " << e.what() << std::endl;
	//}
}

void Input::filterNetlistData(Input& input) {
	//Declare and initialize necessary variables
	int relayCounter = 0;

	//Loops through the netlist data to find the IO expander connections
	for (int i = 0; i < input.netlistData.size(); i++) {
		for (int j = 0; j < input.netlistData[i].name.size(); j++) {
			//Checks if the current line has a numbered connection to the IO expander
			if (input.netlistData[i].device[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && all_of(input.netlistData[i].connection[j].begin(), input.netlistData[i].connection[j].end(), ::isdigit)) {
				//Checks if the connection is a relay based connection
				switch (stoi(input.netlistData[i].connection[j])) {
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
					//Indicates that the information group is not relevant and breaks out of the switch
					break;
				default:
					//Indicates that the information group is relevant and assigns the data to relay list
					input.relayList.push_back(NetlistData());

					//Allocates and initializes data into the relay list from the full netlist data
					for (int m = 0; m < input.netlistData[i].name.size(); m++) {
						input.relayList[relayCounter].name.push_back(input.netlistData[i].name[m]);
						input.relayList[relayCounter].identifier.push_back(input.netlistData[i].identifier[m]);
						input.relayList[relayCounter].connection.push_back(input.netlistData[i].connection[m]);
						input.relayList[relayCounter].device.push_back(input.netlistData[i].device[m]);
					}

					//Breaks out of the switch
					break;
				}
			}

			//Breaks out of the loop if a new relay has been found
			if (relayCounter < input.relayList.size()) {
				relayCounter++;
				break;
			}
		}
	}

	return;
}

void Input::getNetlistData(Input& input) {
	//Declare and initialize necessary variables
	std::ifstream netlist(input.netlistName);
	std::string holder;
	NetlistData temp;
	std::string comparator = " ";
	int loopCounter = 0;

	//Outputs the name of the netlist file
	std::cout << "Netlist File Path: " << input.netlistName << std::endl;

	//Loops while obtaining data from the netlist line by line
	while (getline(netlist, holder)) {
		holder = toUpper(holder);
		//Checks if this is the first line, compares to see if on the same relay, adjusts necessary information for proper data storage
		if (comparator != " " && comparator.substr(0, comparator.find(" ")) != holder.substr(0, holder.find(" "))) {
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

		//Stores the relevant information in netlistData
		input.netlistData.push_back(NetlistData());
		input.netlistData[loopCounter].name.push_back(temp.name[0]);
		input.netlistData[loopCounter].identifier.push_back(temp.identifier[0]);
		input.netlistData[loopCounter].connection.push_back(temp.connection[0]);
		input.netlistData[loopCounter].device.push_back(temp.device[0]);

		//Removes the temporary data to prepare for the next cycle;
		temp.name.clear();
		temp.identifier.clear();
		temp.connection.clear();
		temp.device.clear();
	}

	return;
}

void Input::getDeviceOrder(Input& input) {
	//Declare and initialize necessary variables
	std::string identifier = "";
	bool connectionFound = false;
	bool lastDevice = false;
	int expanders = 0;

	input.ioData.push_back(IOData());

	//Finds the first link in the daisy chain of IO expanders by looping through each piece of netlist data
	for (int i = 0; i < input.netlistData.size(); i++) {
		//Loops through each piece of data associated with the name stored in netlist data
		for (int j = 0; j < input.netlistData[i].name.size(); j++) {
			//Checks if the data has an IO expander connection to DIN
			if (input.netlistData[i].device[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[i].connection[j] == "33" && identifier == "") {
				identifier = input.netlistData[i].identifier[j];
			}
			//Checks if the data has an IO expander connection to DOUT
			else if (input.netlistData[i].device[j] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[i].connection[j] == "40") {
				identifier = "OUTPUT";

				//Exits the loop since we are only searching for the first link, if an output is associated it cannot be the first link
				break;
			}
		}
		//Checks if the data had an IO expander DIN connection but no DOUT connection
		if (identifier != "" && identifier != "OUTPUT") {
			//Stores the information for the first IO expander
			input.ioData[expanders].identifier = identifier;
			input.ioData[expanders].position = expanders + 1;

			//Increments the amount of expanders and breaks out of the loop
			expanders++;
			break;
		}
		//Resets the identifier for the next cycle
		identifier = "";
	}

	//Loops through all of netlist data until all IO expanders are found
	while (!lastDevice) {
		//Loops through each piece of data associated with the name stored in netlist data
		for (int k = 0; k < input.netlistData.size(); k++) {
			for (int l = 0; l < input.netlistData[k].name.size(); l++) {
				//Checks if the data has an IO expander connection to DOUT
				if (input.netlistData[k].device[l] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[k].connection[l] == "40" && input.netlistData[k].identifier[l] == input.ioData[expanders - 1].identifier) {
					//Searches for the corresponding DIN
					for (int m = 0; m < input.netlistData[k].name.size(); m++) {
						//If a corresponding DIN is found, the information is obtained
						if (input.netlistData[k].device[m] == "MAX6957ATL_TQFN-40-MAX6957ATL+" && input.netlistData[k].connection[m] == "33") {
							//Stores the connection information
							input.ioData.push_back(IOData());
							input.ioData[expanders].identifier = input.netlistData[k].identifier[m];
							input.ioData[expanders].position = expanders + 1;

							//Increments the index of the list of IO expanders, states that the daisy chain continues, breaks out of the loop
							expanders++;
							connectionFound = true;
							break;
						}
					}
					if (!connectionFound) {
						//If no corresponding DIN is found, the daisy chain is complete, breaks out of the loop
						lastDevice = true;
						break;
					}

					//Resets the connection found variable
					connectionFound = false;
				}
			}
			//Breaks out of the loop if the daisy chain is complete
			if (lastDevice) {
				break;
			}
		}
	}

	return;
}

void Input::getRelayModels(Input& input) {
	//This function will be modified to get the relay information from the data base. For now it takes in the hard coded information
	//Declares and initializes necessary variables
	std::vector<ModelData> temp = { {"9901-05-20", "CONFIG_LED_18p0_mA"}, {"G3VM-101QR1", "CONFIG_LED_10p5_mA"}, {"G3VM-41QR10", "CONFIG_LED_10p5_mA"},
									{"AQY225R3TY", "CONFIG_LED_10p5_mA"}, {"120-1-A-5/1", "CONFIG_LED_21p0_mA"}, {"50935", "CONFIG_LED_21p0_mA"} };

	/*
	Coto 9901-05-20 --- 18.0mA
	Omron G3VM-101QR1 --- 10.5mA
	Omron G3VM-41QR10 --- 10.5mA
	Panasonic AQY225R3TY --- 10.5mA
	Pickering 120-1-A-5/1 --- 21.0mA
	Tyco 50935 --- 21.0mA
	*/

	//Stores the relay models from the temporary data into model data
	for (int i = 0; i < temp.size(); i++) {
		input.modelData.push_back(temp[i]);
	}

	//Return to main
	return;
}

void Input::getInputData(Input& input) {
	//This function retrieves the relay model information from the database
	input.getRelayModels(input);

	//This function retrieves the bill of materials data from the BOM file
	input.getBillData(input);

 	//This function retrieves the netlist data from the netlist file
 	input.getNetlistData(input);
 
 	//This function filters through the netlist data to determine the relay list
 	input.filterNetlistData(input);
 
 	//The function retrieves the order of the IO expanders
 	input.getDeviceOrder(input);

	//Return to main
	return;
}

std::string Input::toUpper(std::string str)
{
	//This function will convert a string to all uppercase
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

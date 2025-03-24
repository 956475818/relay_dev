#include "Output.h"

//This function will eventually be used to send intermediate data to the database so that it can be accessed without reupload to finish the process
//void Output::writeIntermediate(const Input input, Output& output){
//    return;
//}

//This function will eventually be used to transfer to relay data from the application tier to the database
void Output::outputRelayData(const Relay& relay, Output& output){
    //Declare and initialize necessary variables
    std::ofstream file(output.outputName);
    
    //Checks if the file was opened successfully
    if(!file.is_open()){
        std::cerr << "Failed to open file!\n";
        return;
    }

    std::cout << "Output File Path: " << outputName << std::endl;

    //Write to the file line by line the relay data in CSV format
    for(int i=0; i<relay.relayAmount; i++){
        file << "RELAY(" << relay.relayData[i].pin << "," << relay.relayData[i].enumeration << "," << relay.relayData[i].chainNumber << "," <<
        relay.relayData[i].deviceNumber << "," << relay.relayData[i].portNumber << "," << relay.relayData[i].driveStrength << "," <<
        relay.relayData[i].normalState << "," << relay.relayData[i].minOpenTime << "," << relay.relayData[i].typOpenTime <<
        "," << relay.relayData[i].maxOpenTime << "," << relay.relayData[i].minCloseTime << "," << relay.relayData[i].typCloseTime <<
        "," << relay.relayData[i].maxCloseTime << "," << relay.relayData[i].relayName << ")";

        if(i < relay.relayAmount - 1){
            file << std::endl;
        }
    }

    //Close the file and output that the file was created successfully
    file.close();
    std::cout << "CSV file created successfully\n";
    
    //Return to main
    return;
}


void Output::saveData(const Input& input, const Relay& relay)
{
	//保存数据到数据库
	// 1. 保存 billData
	try {
		std::string insertQuery = "INSERT INTO InputBOM (CIB_ID, ITEM, QTY, MFG, MFG_PART_NUM, REF_DES, DESCRIPTION, DNI, STATUS, MAX_TEMP) VALUES (?,?,?,?,?,?,?,?,?,?)";
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(insertQuery));
		for (const auto& bill : input.billData) {
			pstmt->setInt(1, 1);
			pstmt->setInt(2, stoi(bill.item));
			pstmt->setInt(3, stoi(bill.quantity));
			pstmt->setString(4, bill.manufacturer);
			pstmt->setString(5, bill.partNumber);
			pstmt->setString(6, bill.identifier[0]);
			pstmt->setString(7, bill.description);
			pstmt->setString(8, bill.dni);
			pstmt->setString(9, bill.status);
			pstmt->setString(10, bill.maxTemp);
			pstmt->execute();
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Database error: " << e.what() << std::endl;
	}

	// 2. 保存 netlistData
	try {
		std::string insertQuery = "INSERT INTO InputNetlist (CIB_ID, net, name, identifier, connection, device) VALUES (?,?,?,?,?,?)";
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(insertQuery));
		for (const auto& netlist : input.netlistData) {
			for (int i = 0; i < netlist.name.size(); i++) {
				pstmt->setInt(1, 1);
				pstmt->setInt(2, 1);
				pstmt->setString(3, netlist.name[i]);
				pstmt->setString(4, netlist.identifier[i]);
				pstmt->setString(5, netlist.connection[i]);
				pstmt->setString(6, netlist.device[i]);
				pstmt->execute();
			}
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Database error: " << e.what() << std::endl;
	}

	// 3. 保存 Output
	try {
		std::string insertQuery = "INSERT INTO OutputConfigFile (CIB_ID, line_num, DUT_PIN, enumeration, chain_num, device_num, port_num, relay_drive, normal_state, "
			"open_min, open_typ, open_max, close_min, close_typ, close_max, descr_name) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(insertQuery));
		for (int i = 0; i < relay.relayAmount; i++) {
			pstmt->setInt(1, 1);
			pstmt->setInt(2, i);
			pstmt->setString(3, relay.relayData[i].pin);
			pstmt->setString(4, relay.relayData[i].enumeration);
			pstmt->setInt(5, relay.relayData[i].chainNumber);
			pstmt->setInt(6, relay.relayData[i].deviceNumber);
			pstmt->setString(7, relay.relayData[i].portNumber);
			pstmt->setString(8, relay.relayData[i].driveStrength);
			pstmt->setString(9, relay.relayData[i].normalState);
			pstmt->setInt(10, relay.relayData[i].minOpenTime);
			pstmt->setInt(11, relay.relayData[i].typOpenTime);
			pstmt->setInt(12, relay.relayData[i].maxOpenTime);
			pstmt->setInt(13, relay.relayData[i].minCloseTime);
			pstmt->setInt(14, relay.relayData[i].typCloseTime);
			pstmt->setInt(15, relay.relayData[i].maxCloseTime);
			pstmt->setString(16, relay.relayData[i].relayName);
			pstmt->execute();
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Database error: " << e.what() << std::endl;
	}
}
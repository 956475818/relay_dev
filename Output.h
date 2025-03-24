#pragma once

#include "CIBData.h"
#include "Input.h"
#include "Relay.h"

class Output {
private:
	std::shared_ptr<sql::Connection> conn;
public:
	explicit Output(const std::shared_ptr<sql::Connection>& conn) : conn(conn) {}
	std::string outputName;
	//std::string intermediateName;
	//void writeIntermediate(const Input&, Output&);
	void outputRelayData(const Relay&, Output&);
	void saveData(const Input&, const Relay&);
};
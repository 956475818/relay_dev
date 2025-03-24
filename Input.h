#pragma once

#include "CIBData.h"

class Input {
    private:
        void getNetlistData(Input&);
        void getBillData(Input&);
        void getDeviceOrder(Input&);
        void filterNetlistData(Input&);
        void getRelayModels(Input&);
    public:
        std::string netlistName;
        std::string billName;
        std::string boardType;
        std::vector<BillData> billData;
        std::vector<IOData> ioData;
        std::vector<NetlistData> netlistData;
        std::vector<NetlistData> relayList;
        std::vector<ModelData> modelData;
        void getInputData(Input&);
        std::string toUpper(std::string str);
};
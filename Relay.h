#pragma once

#include "CIBData.h"
#include "Input.h"

class Relay {
    private:
        std::string convertPin(const Input&, const Relay&);
        std::string convertEnum(const Input&, const Relay&);
        int convertChain(const Input&);
        int convertDevice(const Input&, const Relay&);
        std::string convertPort(const Input&, const Relay&);
        std::string convertDriveStrength(const Input&, const Relay&);
        std::string convertNormalState(const Input&, const Relay&);
        int deviceModifier;
        int currentRelay;
    public:
        std::vector<RelayData> relayData;
        int relayAmount;
        void relayConvert(const Input&, Relay&);
};
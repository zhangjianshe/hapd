//
// Created by Xule Zhou on 8/7/18.
//

#ifndef ARDUINOHOMEKIT_MANUFACTURER_H
#define ARDUINOHOMEKIT_MANUFACTURER_H

#include "GenericCharacteristic.h"

class Manufacturer: public GenericCharacteristic<0x00000020, CPERM_PR, FORMAT_STRING, const char *>{
public:
    explicit Manufacturer(HAPServer *server) : GenericCharacteristic(server) {
        value.string_value = "HomeKitManufacturer";
    }
};

#endif //ARDUINOHOMEKIT_MANUFACTURER_H
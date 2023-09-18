#include "gb/AddressBus.h"
#include "gb/memory/Memory.h"
#include "utils/Utils.h"

#include <string>
#include <sstream>
#include <exception>
#include <algorithm>


namespace gb {

    bool less(MemoryController mem, uint16_t address) {
        return mem.getMaxAddress() < address;
    }

    uint8_t AddressBus::read(uint16_t address) const
    {
        auto it = std::lower_bound(memory_.begin(), memory_.end(), address, less);
        if (it == memory_.end() || it->getMinAddress() > address)
        {
            throw std::out_of_range(getErrorDescription(address));
        }

        if(!observers_.empty())
        {
            auto obs_it = std::lower_bound(observers_.begin(), observers_.end(), address, less);
            if(obs_it != observers_.end() && address >= obs_it->getMinAddress())
            {
                obs_it->read(address);
            }
        }
        
        return it->read(address);
    }

    void AddressBus::write(uint16_t address, uint8_t data) const
    {
        auto it = std::lower_bound(memory_.begin(), memory_.end(), address, less);
        if (it == memory_.end() || it->getMinAddress() > address)
        {
            throw std::out_of_range(getErrorDescription(address));
        }

        if(!observers_.empty())
        {
            auto obs_it = std::lower_bound(observers_.begin(), observers_.end(), address, less);
            if(obs_it != observers_.end() && address >= obs_it->getMinAddress())
            {
                obs_it->write(address, data);
            }
        }

        it->write(address, data);

    }
    std::string AddressBus::getErrorDescription(uint16_t address, int value) const
    {
        std::stringstream err;
        
        if (value == -1)
        {
            err << "Attempting to read from invalid memory address: ";
            toHexOutput(err, address);
        }
        else
        {
            err << "Attempting to write to invalid memory address: ";
            toHexOutput(err, address);
            err << ". Data: ";
            toHexOutput(err, value);
        }

        return err.str();
    }

    void AddressBus::connect(const MemoryController& controller) {
        auto it = std::lower_bound(memory_.begin(), memory_.end(), controller);
        memory_.insert(it, controller);
    }

    void AddressBus::addObserver(const MemoryController& observer) {
        auto it = std::lower_bound(observers_.begin(), observers_.end(), observer);
        observers_.insert(it, observer);
    }
}

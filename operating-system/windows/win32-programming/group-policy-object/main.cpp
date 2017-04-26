
#include "GPOManager.h"
#include <iostream>

int main()
{
    try{
        GPOManager manager;
        manager.applyPolicy(GPOManager::GPOType::USB_DEVICE_REMOVE, false);
    }
    catch (GPOException& e)
    {
        std::cerr << e.what() << std::endl;
        exit(-1);
    }
    
    std::cout << "apply a policy success" << std::endl;
    return 0;
}
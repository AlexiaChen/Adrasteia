#include "BrowserInfo.h"
#include <iostream>
int main()
{
    std::cout << ActiveTab::url(ActiveTab::BrowserType::FIRE_FOX) << std::endl;
    return 0;
}


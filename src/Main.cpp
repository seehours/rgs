#include "Application.h"

#include "Math.h"
#include <iostream>

int main ()
{
    std::cout << "Hello RGS!" << std::endl;
    RGS::Application app("RGS", 400, 300);
    app.Run();
    std::cout << "Bye 0Bao!" << std::endl;
}
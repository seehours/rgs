#include "RGS/Application.h"

#include "RGS/Maths.h"
#include "RGS/Shaders/BlinnShader.h"

#include <iostream>

int main()
{
    std::cout << "Hello RGS!" << std::endl;
    RGS::Application app("RGS", 800, 600);
    app.Run();
    std::cout << "Bye 0Bao!" << std::endl;
}
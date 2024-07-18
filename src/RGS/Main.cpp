#include "RGS/Application.h"

#include "RGS/Maths.h"
#include "RGS/Shaders/BlinnShader.h"

#include <iostream>

constexpr int test = 9;

int main()
{
    std::cout << "Hello RGS!" << std::endl;
    RGS::Application app("RGS", 400, 300);
    app.Run();
    std::cout << "Bye 0Bao!" << std::endl;
}
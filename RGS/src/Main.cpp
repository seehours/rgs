#include "Application.h"

int main() 
{
#if 0
    RGS::Application app("Viewport", 400, 300);
#else
    RGS::Application app("Viewport", 800, 600);
#endif
    app.Run();

    return 0;
}
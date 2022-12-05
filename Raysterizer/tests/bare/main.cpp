#include <Windows.h>
int main()
{
    auto lib = LoadLibrary("../../opengl32.dll");
    while (1)
    {
        Sleep(100000000);
    }
    return 0;
}

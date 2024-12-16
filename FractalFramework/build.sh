g++ -o FractalFramework.exe FractalFramework.cpp -Wall -fopenmp -std=c++17 -O3 -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static  


# MinGW
#g++ -o olcExampleProgram.exe olcExampleProgram.cpp -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -std=c++17

# Linux
#g++ -o olcExampleProgram olcExampleProgram.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17

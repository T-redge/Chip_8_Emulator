@echo off
gcc -o main.exe src/main.c src/chip8.c -IC:\SDL2-2.30.11\x86_64-w64-mingw32\include\SDL2 -LC:\SDL2-2.30.11\x86_64-w64-mingw32\lib -lmingw32 -lSDL2main -lSDL2 -Wall -Wextra -Wpedantic
if %errorlevel% == 0 (
	echo success
	main.exe
) else (
	echo failed
)
pause
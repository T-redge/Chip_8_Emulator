@echo off
gcc -o main.exe main.c stack.c -ID:\extlib\SDL2\include\SDL2 -LD:\extlib\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -Wall -Wextra -Wpedantic
if %errorlevel% == 0 (
	echo success
	main.exe
) else (
	echo failed
)
pause
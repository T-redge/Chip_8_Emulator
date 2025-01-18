@echo off
gcc -o main.exe main.c -g -Wall -Wextra -Wpedantic
if %errorlevel% == 0 (
	echo success
	main.exe
) else (
	echo failed
)
pause
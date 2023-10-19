@ECHO OFF
gcc -c scheduling.c functions.c
gcc scheduling.o functions.o functions.h -o main
main
ECHO.
pause
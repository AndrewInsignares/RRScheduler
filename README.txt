README:

Description:
This project was made to test the performance of a Round Robin scheduling system while handling trap interrupts. The scheduler keeps count of active processes and terminates when no more processes left. Runtime and trap handlers can be seen in the file “system” or “system.exe” on Windows. This can be ran from the GitHub below with code located in kernel.c 

Run:

GCC compiler needed

In directory of files:

$: gcc –m32 –o system kernel.c hardware.o drivers.o

Output:

	Windows:
	system.exe
	
	Mac:
	system

To run the part1 module and program simply enter
	make
or
	make all

Then insert the module using

	sudo insmod part1.ko

to test the module use 
./test1 
to check the intercepted system call 1

and then to check the virus scanner, either swap to a user profile and open the test.txt or enter this helpful command:

	sudo -u [user profile name] cat "test.txt"

and check the syslog to view all outputs.

use 
	sudo rmmmod part1.ko 

to remove it

and use 
	make clean

to empty the directory!
	

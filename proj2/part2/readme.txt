Kernel Module Interception:
	Compile by using:
		make 
	or 
		make all

	to clean use:
		make clean

	then insert using:
		sudo insmod kernsysinfo.ko

	NOTE: procAncestry will not work if this is not inserted!!

	to remove use:
		sudo rmmod kernsysinfo.ko

This module outputs to the kernel syslog, therefore to find more information either run the complementary program procAncestry or check the syslog to see output. 

procAncestry:
	Compile by using:
		make
	or
		make all
	
	to clean use:
		make clean

	to run use:
		./a.out x [where x is the PID the user is looking for]
	
proAncestry outputs to the terminal. It will print out the siblings, children, and ancestors of the process given in the user input line.

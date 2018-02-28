Daniel Sullivan (djsullivan) and Przemek Gardias (pmgardias)

GENERAL

This program is a virtual memory manager. It simulates memory being mapped, stored, loaded, and swapped. It is composed of a singular mng.c file. The program creates a disk.txt file when run to use to emulate a disk space. 

To compile the program, the user must go to the directory of the file in the terminal and type

	make

which will then create an executable ./mng file. This program does not take any command line arguments and runs as soon as the executable is run. From here, it will prompt for user input by printing "Instruction?". Here, the user must put their instructions in the form of a tuple such that

	w,x,y,z

where w is the process ID, x is the instruction type (map, store, load), y is the virtual address, and z is the value for the instruction. Process ID must be in the range of [0,3], virtual address must be in the range of [0,63] and value must be in the range of [0,255].

To clean the file, the user must type in 

	make clean

which will remove all the files used to make the executable. This program also supports the command 

	make clean all

which cleans the folder and proceeds to compile and create the executable after cleaning.

TESTING

To test our program, we have four test cases that test the abilities of our meory management simulation. To test the program, we test it with different command options, multiple processes, and check against incorrect inputs.

To run the tests, pipe them as the input file, such as 

	./p4 < test1.txt

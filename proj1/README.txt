Przemek Gardias (pmgardias) & Daniel Sullivan (djsullivan)

Background Processes:
Background processes are kept track of through a structure of type pinfo_t, which contains information about the process such as the name, process id number, starting time of type timeval, and resource usage of the process of type rusage. The process info is held in an array, and modified through functions newBgProcess() and removeBgProcess(). Functions which need access to the array of background processes, such as userOp() and alphOp(), use the array in a read-only manner.

Testing Process:
We tested our program by attempting many possible variations of user inputs, including invalid ones. Some commands, such as 0 (whoami) and 1 (last) did not have many options in testing, however others such as 2 (ls) had more. These commands were tested more thoroughly, with multiple variances in arguments passed. User-added commands were also tested thoroughly, both as foreground and background processes. Background jobs of different runtime durations were specifically tested in order to asceratain that the program supports out-of-order background processes.

Testing Command:
To test Mid Day Commander, enter "./mcX < test1.txt" into the shell, replacing the X with the version of Mid Day Commander which is being run, this will allow the program to take input from the file test1.txt. This command can be repeated for all other test cases which are bundled with the program. Some versions (mc1 & mc2) of Mid Day Commander include multiple test files. If output should be written to a text file, the command format "./mcX < test1.txt &> output1.txt" can be used.

Outputs:
The files named outputX.txt are the output of their respective test, testX.txt. They are fairly intuitive to understand, however lack the input which the tests passed to the program. This input would normally visible to the user if the program was manually run through a shell. For v2 of Mid Day Commander, the ouput of the tests is very long and contains numerous error messages that the program is unable to quit because background processes are still running. This is because the shell is in a while loop which repeatedly tried to take the EOF input from the test file, to then quit the program.

Notes:
- In the case of no arguments passed, command 2 (ls) defaults to the flag -al. To enter no arguments as the user, simply Enter when prompted for "Arguments?:".
- usleep() is used at the beginning of the main while loop that the program is based on, in order to minimize the amount of errors stating that the program is unable to quit because of background processes. This greatly reduces the otherwise near-infinite redundant output that would be written to the output files.


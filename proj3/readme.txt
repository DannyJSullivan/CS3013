To compile simply use 
	make
Also supports make clean!

To run use:
	./bathroomsim users averagearrival averageusetime averageloops

where:
-users: the number of users to simulate, more specifically how many threads to generate
-averagearrival: The average time it takes for a thread(user) to arrive at the bathroom
-averageusetime: The average amount of time a thread(user) is in the bathroom for
-averageloops: The average number of loops a thread will run, or how many times it will use the bathroom

It prints out statistics from each individual thread when it terminates on the amount of time it was in queue for, and if it didn't all the stats are zero. Then when all the threads terminate the main thread prints out the data for the bathroom and queue usage statistics. 

Source File Contents:

Bathroom.c: Contains the functions for the bathroom simulation, from starting to ending the simulation and entering and exiting the bathroom. Along with a function to generate standardized random numbers.

Bathroomsim.c: Contains the global bathroom, the indvidiual function passed to threads using functions from bathroom.c. Most importantly bathroomsim runs the entire simulation through main with command line arguments.

TEST CASES:
can be run by simply entering:
	./test1 or ./test2 or ./test3
If any seg fault it is not because of the program, rather the number of threads created may be too large for certain machines. It will print out the command line it ran at the end.

Keeping track of the statistics:
	To track the time that the bathroom is vacant, we start a timer at the initialization of the 		bathroom as well as a timer for whenever the bathroom is marked vacant through a flag. We then add that number to the total vacant time, a field in our bathroom struct. Then at the end we simply subtract the vacant time from the total time to calculate the time the bathroom was occupied as this is the only other state. 
	To track the statistics of average queue length and average number of users in the bathroom we simply use a discrete probablility distribution. Since the values of either these can only be integers since they are whole people, so we define two arrays qarray and barray that are the size of the input number of users. Then everytime anyone enters the bathroom or enters the queue we simply increment that position in the array. Then at the end we loop through the entire array and calculate the total frequency. Then loop back through and multiply each index with the percentage it contains which is calucluated as array[i]/total frequency. This is the statistical formula for expected value or E(x) or mean. 



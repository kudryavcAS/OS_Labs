Task. Write a program for a console process that consists of three threads: main, min_max, and average. 

The main thread must perform the following actions:

	1) Create an array of integers, whose size and elements are entered from the console.
	2) Create the min_max and average threads.
	3) Wait for the min_max and average threads to complete.
	4) Replace the maximum and minimum elements of the array with the average value of the array elements. Print the results to the console.
	5) Terminate.

The min_max thread must perform the following actions:

    1) Find the minimum and maximum elements of the array and print them to the console. After each element comparison, "sleep" for 7 milliseconds.
	2) Terminate.
	
The average thread must perform the following actions:

    1) Find the arithmetic mean of the array elements and print it to the console. After each element summation operation, "sleep" for 12 milliseconds.
	2) Terminate.

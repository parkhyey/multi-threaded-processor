# Multi-threaded Producer Consumer Pipeline
Text Line Processor; Multi-threaded Producer Consumer Pipeline

## Instructions
Create a program that creates 4 threads to process text input from standard input as follows
Thread 1, called the Input Thread, reads in lines of characters from the standard input.
Thread 2, called the Line Separator Thread, replaces every line separator in the input by a space.
Thread 3, called the Plus Sign thread, replaces every pair of plus signs, i.e., "++", by a "^".
Thread 4, called the Output Thread, write this processed data to standard output as lines of exactly 80 characters.
Furthermore, these 4 threads must communicate with each other using the Producer-Consumer approach. 

## Multi-Threading Requirements
![image](https://user-images.githubusercontent.com/71689421/168721083-2bf7fbcb-7186-40a2-aae4-ab92837661fd.png)

A pipeline of 4 threads. 
Input thread reads data from stdin and puts it in Buffer 1. 
Line Separator thread gets data from Buffer 1, processes it and puts it in Buffer 2. 
Plus Sign Thread reads data from Buffer 2, processes it and puts it in Buffer 3. 
Output Thread reads data from Buffer 3 and output lines to stdout.

Pipeline of threads that gets data from stdin, processes it and displays it to stdout.

Each pair of communicating threads must be constructed as a producer/consumer system.
If a thread T1 gets its input data from another thread T0, and T1 outputs data for use by another thread T2, then T1 acts as a consumer with respect to T0 and T0 plays the role of T1’s producer T1 acts as a producer with respect to T2 and T2 plays the role of T1’s consumer. Thus each thread in the interior of the pipeline (i.e., the Line Separator and Plus Sign threads) will contain both producer code and consumer code. Each producer/consumer pair of threads will have its own shared buffer. Thus, there will be 3 of these buffers in your program, each one shared only by its producer and consumer.
It must use synchronization variables for coordination. The program must never sleep.

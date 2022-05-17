/* ************************************************************************** */
/* Author: Hye Yeon Park                                                      */
/* Date: 2/18/2022                                                            */
/* Project: Line Processor (Multi-threaded Producer Consumer Pipeline)        */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

// Size of the buffers
#define SIZE 50

// Number of items that will be produced.
// This number is less than the size of the buffer. (Unbounded buffer)
#define NUM_ITEMS 49
#define NUM_CHAR 1000

// Buffer 1, shared resource between input thread and line_separator thread
char* buffer_1[SIZE];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the line_separator thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

// Buffer 2, shared resource between line_separator thread and plus_sign thread
char* buffer_2[SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the line_separator thread will put the next item
int prod_idx_2 = 0;
// Index where the plus_sign thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// Buffer 3, shared resource between plus_sign thread and output thread
char* buffer_3[SIZE];
// Number of items in the buffer
int count_3 = 0;
// Index where the plus_sign thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

// Stores remaining string of the input
char remaining_line[NUM_CHAR + 81];

/* ************************************************************************** */
/* Function: put_buff_1()                                                     */
/* Description: Put an item in buff_1                                         */
/* Reference: replit.com/@cs344/65prodconspipelinec                           */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
void put_buff_1(char* line)
{
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_1);
	// Put the item in the buffer
	buffer_1[prod_idx_1] = line;
	// Increment the index where the next item will be put.
	prod_idx_1 = prod_idx_1 + 1;
	count_1++;
	// Signal to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_1);
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_1);
}

/* ************************************************************************** */
/* Function: get_input()                                                      */
/* Description: Function that the input thread will run; Get input from user. */
/*              Put the item in the buffer shared with line_separator thread. */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
void* get_input(void* args)
{
	while (1)
	{
		// Get the user input
		char* line = malloc(sizeof * line * NUM_CHAR);
		fgets(line, sizeof * line * NUM_CHAR, stdin);
		// Set the line to NULL if STOP string is passed
		if (strncmp(line, "STOP\n", 5) == 0)
		{
			line = NULL;
			// Put the line in the buffer 1 shared with line_separator thread
			put_buff_1(line);
			// Exit function
			return NULL;
		}
		else
		{
			put_buff_1(line);
		}
	}
	return NULL;
}

/* ************************************************************************** */
/* Function: get_buff_1()                                                     */
/* Description: Get the next item from buffer 1                               */
/* Reference: replit.com/@cs344/65prodconspipelinec                           */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
char* get_buff_1()
{
	char* line;
	// Lock the mutex before checking if the buffer has data
	pthread_mutex_lock(&mutex_1);
	while (count_1 == 0)
		// Buffer is empty. Wait for the producer to signal that the buffer has data
		pthread_cond_wait(&full_1, &mutex_1);
	line = buffer_1[con_idx_1];
	// Increment the index from which the item will be picked up
	con_idx_1 = con_idx_1 + 1;
	count_1--;
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_1);
	// Return the item
	return line;
}

/* ************************************************************************** */
/* Function: put_buff_2()                                                     */
/* Description: Put an item in buff_2                                         */
/* Reference: replit.com/@cs344/65prodconspipelinec                           */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
void put_buff_2(char* line)
{
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_2);
	// Put the item in the buffer
	buffer_2[prod_idx_2] = line;
	// Increment the index where the next item will be put.
	prod_idx_2 = prod_idx_2 + 1;
	count_2++;
	// Signal to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_2);
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_2);
}

/* ************************************************************************** */
/* Function: replace_line_separator()                                         */
/* Description: Funtion that the line_separator thread will run. Replace a ne */
/*              w line character with a space. Get the item from the buffer s */
/*              hared with the input thread. Put the item in the buffer share */
/*              d with the plus_sign thread.                                  */
/* ************************************************************************** */
void* replace_line_separator(void* args)
{
	char* line;
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		// Get line from the buffer 1 shared with input thread
		line = get_buff_1();
		if (line != NULL)
		{
			// Replace the line separator with a space
			line[strlen(line) - 1] = ' ';
			// Put the line in the buffer 2 shared with plus_sign thread
			put_buff_2(line);
		}
		else
		{
			put_buff_2(line);
			return NULL;
		}
	}
	return NULL;
}

/* ************************************************************************** */
/* Function: get_buff_2()                                                     */
/* Description: Get the next item from buffer 2                               */
/* Reference: replit.com/@cs344/65prodconspipelinec                           */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
char* get_buff_2()
{
	char* line;
	// Lock the mutex before checking if the buffer has data
	pthread_mutex_lock(&mutex_2);
	while (count_2 == 0)
		// Buffer is empty. Wait for the producer to signal that the buffer has data
		pthread_cond_wait(&full_2, &mutex_2);
	line = buffer_2[con_idx_2];
	// Increment the index from which the item will be picked up
	con_idx_2 = con_idx_2 + 1;
	count_2--;
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_2);
	// Return the item
	return line;
}

/* ************************************************************************** */
/* Function: put_buff_3()                                                     */
/* Description: Put an item in buff_3                                         */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
void put_buff_3(char* line)
{
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_3);
	// Put the item in the buffer
	buffer_3[prod_idx_3] = line;
	// Increment the index where the next item will be put.
	prod_idx_3 = prod_idx_3 + 1;
	count_3++;
	// Signal to the consumer that the buffer is no longer empty
	pthread_cond_signal(&full_3);
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_3);
}

/* ************************************************************************** */
/* Function: replace_plus_sign()                                              */
/* Description: Funtion that the plus_sign thread will run. Replace double pl */
/*              us signs(++) to caret sign(^). Get the item from the buffer s */
/*              hared with the line_separator thread. Put the item in the buf */
/*              fer shared with the output thread.                            */
/* ************************************************************************** */
void* replace_plus_sign(void* args)
{
	char* line;
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		// Get the line from the buffer 2 shared with line_separator
		line = get_buff_2();
		if (line != NULL)
		{
			// Set a buffer to hold revised string
			char buffer[NUM_CHAR];
			char* buffer_ptr = buffer;
			char* line_ptr = line;
			while (1)
			{
				// Search ++ from line
				char* temp = strstr(line_ptr, "++");
				// If no match, copy rest of the string
				if (temp == NULL)
				{
					strcpy(buffer_ptr, line_ptr);
					break;
				}
				// Copy substring before ++ and move pointer
				int substr_len = temp - line_ptr;
				memcpy(buffer_ptr, line_ptr, substr_len);
				buffer_ptr += temp - line_ptr;

				// Add the new character ^ and move pointer
				memcpy(buffer_ptr, "^", 1);
				buffer_ptr += 1;
				// Move line pointer
				line_ptr = temp + 2;
			}
			// Copy it back to line
			strcpy(line, buffer);
			// Put the line in the buffer 3 shared with output thread
			put_buff_3(line);
		}
		else
		{
			put_buff_3(line);
			return NULL;
		}
	}
	return NULL;
}

/* ************************************************************************** */
/* Function: get_buff_3()                                                     */
/* Description: Get the next item from buffer 3                               */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
char* get_buff_3()
{
	char* line;
	// Lock the mutex before checking if the buffer has data
	pthread_mutex_lock(&mutex_3);
	while (count_3 == 0)
		// Buffer is empty. Wait for the producer to signal that the buffer has data
		pthread_cond_wait(&full_3, &mutex_3);
	line = buffer_3[con_idx_3];
	// Increment the index from which the item will be picked up
	con_idx_3 = con_idx_3 + 1;
	count_3--;
	// Unlock the mutex
	pthread_mutex_unlock(&mutex_3);
	// Return the item
	return line;
}

/* ************************************************************************** */
/* Function: write_output()                                                   */
/* Description: Function that the output thread will run. Write the processed */
/*              data to standard output as lines of exactly 80 characters. Ge */
/*              t the item from the buffer shared with the plus_sign thread.  */
/*                                                                            */
/* ************************************************************************** */
void* write_output(void* args)
{
	char* line;
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		// Get the line from the buffer 3 shared with plus_sign thread
		line = get_buff_3();
		if (line != NULL)
		{
			// Concatenate the new line with the remaining line
			strcat(remaining_line, line);
			// Redirect the line pointer
			line = remaining_line;
			size_t count = strlen(line);
			// Display output for every 80 characters
			while (count >= 80)
			{
				char output_line[256];
				// Copy 80 characters
				memcpy(output_line, line, 80);
				puts(output_line);
				// Move the pointer
				line += 80;
				count -= 80;
			}
			// Store the remaining line
			strcpy(remaining_line, line);
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}

/* ************************************************************************** */
/* Function: main()                                                           */
/* Description: main function that creates and joins threads                  */
/* Reference: replit.com/@cs344/65prodconspipelinec                           */
/*                                                                            */
/*                                                                            */
/* ************************************************************************** */
int main(int argc, char* argv[])
{
	srand(time(0));
	pthread_t input_t, line_separator_t, plus_sign_t, output_t;

	// Create the threads
	pthread_create(&input_t, NULL, get_input, NULL);
	pthread_create(&line_separator_t, NULL, replace_line_separator, NULL);
	pthread_create(&plus_sign_t, NULL, replace_plus_sign, NULL);
	pthread_create(&output_t, NULL, write_output, NULL);

	// Wait for the threads to terminate
	pthread_join(input_t, NULL);
	pthread_join(line_separator_t, NULL);
	pthread_join(plus_sign_t, NULL);
	pthread_join(output_t, NULL);
	return EXIT_SUCCESS;
}

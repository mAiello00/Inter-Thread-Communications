/*
Marco Aiello
251 047 101
CS 3305 Assignment 2
Due: October 25, 2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

int lenOne;
int lenTwo;
int port[2];

void *sum(void *thread_id)
{
	//get the id of the thread
	int *id = (int *)thread_id;

	//create variables to store the values we read from the pipe
	char charOne[lenOne];
	char charTwo[lenTwo];
	char space[1];

	//read from the pipe and store what is read
	read(port[0], &charOne, lenOne);
	read(port[0], &space, 1);
	read(port[0], &charTwo, lenTwo);

	printf("thread (TID %d) reads X = %s and Y = %s from the pipe\n", *id, charOne, charTwo);

	//convert the characters we read from the pipe to integers so we can sum them
	int numOne = atoi(charOne);
	int numTwo = atoi(charTwo);
	//printf("num one: %d. num two: %d\n", numOne, numTwo);

	int S = numOne + numTwo;

	char c[100];

	//convert the sum to a string
	sprintf(c, "%d", S);
	//printf("this is the char: %s\n", c);

	//get the length of the string
	int charLen = strlen(c);
	//printf("length of the char: %d\n", charLen);

	//write the sum to the port
	write(port[1], &c, charLen);
	printf("thread (TID %d) writes X + Y = %d to the pipe\n", *id, S);

}

void *odd_even(void *thread_id)
{
	//get the id of the thread
	int *id = (int *)thread_id;

	char num[1025];
	int n ;

	if((n = read(port[0], num,1024)) >= 0)
	{
		num[n] = 0;
		printf("thread (TID %d) reads X + Y = %s from the pipe\n",*id, num);
	}
	else
	{
		perror("there was an error reading the sum\n");
		exit(0);
	}

	int sum = atoi(num);

	if((sum % 2) == 0)
	{
		 printf("thread (TID %d) identifies X + Y = %d as an even number\n",*id, sum);
	}
	else
	{
		 printf("thread (TID %d) identifies X + Y = %d as an odd number\n",*id, sum);
	}

	//write the sum back into the pipe so thread_3 can  use it
	write(port[1], &num, strlen(num));
	printf("thread (TID %d) writes X + Y = %d to the pipe\n", *id, sum);

}

void *digit_count(void *thread_id)
{
	//get the id of thread_3
	int *id = (int *)thread_id;

	char num[1025];
	int n;

	if((n = read(port[0], num,1024)) >= 0)
	{
		num[n] = 0;
		printf("thread (TID %d) reads X + Y = %s from the pipe\n", *id, num);
		printf("thread (TID %d) identifies X + Y = %s as a %d digit number\n",*id, num, n);
	}
	else
	{
		printf("there was an error reading from the pipe\n");
		exit(0);
	}
}

int main(int argc, char **argv)
{
	pthread_t thread_1, thread_2, thread_3;
	int thread_1_ID = 100, thread_2_ID = 101, thread_3_ID = 102;

	//get the command line arguments from the user
	char* X = argv[1];
	char* Y = argv[2];

	printf("parent (PID %d) receives X = %s and Y = %s from the user\n",getpid(), X, Y );

	//create the pipe
	//if unsuccessful, tell the user and kill the program
	if(pipe(port) < 0)
	{
		perror("pipe error\n");
		exit(0);
	}

	//create the first thread
	pthread_create(&thread_1, NULL, sum, &thread_1_ID);

	//get the character length of each of the parameters passed by the user
	lenOne = strlen(X);
	//printf("Length of the first parameter: %d\n", lenOne);
	lenTwo = strlen(Y);
	//printf("Length of the second parameter: %d\n", lenTwo);

	//concatenate the two parameters passed by the user into a single string
	int totalConcLen = lenOne + lenTwo ;
	char conc[totalConcLen + 1];
	strcpy(conc, X);
	strcat(conc, " ");
	strcat(conc, Y);

	//write the concatenated parameters to the pipe
	write(port[1], &conc, totalConcLen+1);
	printf("parent (PID %d) writes X = %s and Y = %s to the pipe\n",getpid(), X, Y);

	pthread_join(thread_1, NULL);

	pthread_create(&thread_2, NULL, odd_even, &thread_2_ID);
	pthread_join(thread_2, NULL);

	pthread_create(&thread_3, NULL, digit_count, &thread_3_ID);
	pthread_join(thread_3, NULL);
}

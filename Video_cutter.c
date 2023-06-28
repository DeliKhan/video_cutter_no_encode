#include <stdio.h>
#include <stdint.h>   //to use fixed-width integers
#include <inttypes.h> //to print fixed-width integers
#include <string.h>   //using this to extract time information
#include <stdlib.h>   // for using free

void extracting_time(char start_time[], int *const timings) // converts HH:MM:SS to {HH, MM, SS}
{
    const char delimiter[2] = ":";
    char *token;
    token = strtok(start_time, delimiter);
    int i = 0;
    while (token)
    {
        // printf("\n%d\n", atoi(token));
        timings[i] = atoi(token);
        token = strtok(NULL, delimiter);
        i++;
    }
    free(token);
}

int main() // gets all user input and executes the other functions
{
    char file_directory[100]; // I'm assuming your file directory is less than 100 characters long
    char start_time[8];
    int timings[3];
    printf("Type path to input video file");
    scanf("%s", file_directory);
    printf("Type the start time of your clipping in HH:MM:SS");
    scanf("%s", start_time);
    extracting_time(&start_time, &timings);
    // printf("Hours:%d\nMinutes:%d\nSeconds:%d\n", *(timings), *(timings + 1), *(timings + 2));
}

#include <stdio.h>
#include <stdint.h>   //to use fixed-width integers
#include <inttypes.h> //to print fixed-width integers
#include <string.h>   //using this to extract time information
#include <stdlib.h>   // for using free

void get_input(char *file_directory, char *start_time)
{
    printf("Type path to input video file");
    fgets(file_directory, 100, stdin);
    printf("Type the start time of your clipping in HH:MM:SS");
    fgets(start_time, 9, stdin);
    // remove \n and trailing zeroes
    if ((strlen(file_directory) > 0) && (file_directory[strlen(file_directory) - 1] == '\n'))
    {
        file_directory[strlen(file_directory) - 1] = '\0';
    }
}
void extracting_time(char *time, uint64_t *start_time_sec) // converts HH:MM:SS to {HH, MM, SS}
{
    const char delimiter[2] = ":";
    uint8_t timings[3];
    char *token = strtok(time, delimiter);
    uint8_t i = 0;
    while (token)
    {
        timings[i] = atoi(token);
        token = strtok(NULL, delimiter);
        i++;
    }
    *start_time_sec = (uint64_t)3600 * timings[0] + 60 * timings[1] + timings[2];
    // printf("%" PRIu64 "\n", *start_time_sec);
}

int get_iframes(uint64_t *start_time_sec, long double *iframes, char *file_directory)
{
    char command[500] = "ffprobe -v error -skip_frame nokey -show_entries frame=pkt_pts_time -select_streams v -read_intervals '";
    char beginning[20];
    char end[20];

    if (*start_time_sec > 180)
    {
        sprintf(beginning, "%" PRIu64, *start_time_sec - 180);
        strcat(command, beginning);
        strcat(command, "%");
    }
    else
    {
        strcat(command, "0%");
    }
    sprintf(end, "%" PRIu64, *start_time_sec + 180);
    strcat(command, end);
    strcat(command, "'");
    strcat(command, " -of csv=p=0 '");
    strcat(command, file_directory);
    strcat(command, "'");
    // remove \n and trailing zeroes
    if ((strlen(command) > 0) && (command[strlen(command) - 1] == '\n'))
        command[strlen(command) - 1] = '\0';
    char line[50];
    size_t chunks;
    FILE *process;
    process = popen(command, "r");
    if (process == NULL)
    {
        printf("Command failed to execute\n");
        return 1;
    }
    uint8_t counter = 0;
    while (fgets(line, sizeof(line), process) != NULL)
    {
        // remove \n and trailing zeroes
        if ((strlen(line) > 0) && (line[strlen(line) - 1] == '\n'))
            line[strlen(line) - 1] = '\0';
        // store Iframes into array
        iframes[counter] = atof(line);
        counter++;
    }
    // reallocate the iframes to save memory
    iframes = (long double *)realloc(iframes, counter * sizeof(long double));
    pclose(process);
    printf("Here are the following iframe positions in seconds: \n");
    for (uint8_t i = 0; i < counter; i++)
    {
        printf("%Lf \n", iframes[i]);
    }
}

int main() // gets all user input and executes the other functions
{
    char file_directory[100]; // I'm assuming your file directory is less than 100 characters long
    char start_time[9] = {'\0'};
    char end_time[9] = {'\0'};
    uint64_t start_time_sec = 0.0;
    long double *iframes = malloc(10000 * sizeof(long double));
    if (iframes == NULL)
    {
        printf(" Memory allocation failed. Exiting the program .\n");
        return 1;
    }
    get_input(file_directory, start_time);
    extracting_time(start_time, &start_time_sec);
    get_iframes(&start_time_sec, iframes, file_directory);
}

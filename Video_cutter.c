#include <stdio.h>
#include <stdint.h>   //to use fixed-width integers
#include <inttypes.h> //to print fixed-width integers
#include <string.h>   //using this to extract time information
#include <stdlib.h>   // for using free

void get_input(char *file_directory, char *destination_path, char *start_time, char *end_time)
{
    printf("Type path to input video file");
    fgets(file_directory, 100, stdin);
    printf("Type the start time of your clipping in HH:MM:SS");
    fgets(start_time, 10, stdin);
    printf("Type the end time of your clipping in HH:MM:SS");
    fgets(end_time, 10, stdin);
    // remove \n and trailing zeroes
    if ((strlen(file_directory) > 0) && (file_directory[strlen(file_directory) - 1] == '\n'))
    {
        file_directory[strlen(file_directory) - 1] = '\0';
    }
    if ((strlen(end_time) > 0) && (end_time[strlen(end_time) - 1] == '\n'))
    {
        end_time[strlen(end_time) - 1] = '\0';
    }
    // find destination path
    char *slash_directory = strrchr(file_directory,'/');
    int index = (int) (slash_directory-file_directory);
    index++;
    destination_path = strndup(file_directory,index);
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

int get_iframes(uint64_t *start_time_sec, long double *iframes, char *file_directory, uint8_t *counter)
{
    char command[500] = "ffprobe -v error -skip_frame nokey -show_entries frame=pkt_pts_time -select_streams v -read_intervals '";
    char beginning[20];
    char end[20];

    if (*start_time_sec > 180)
    {
        sprintf(beginning, "%" PRIu64, *start_time_sec - 120);
        strcat(command, beginning);
        strcat(command, "%");
    }
    else
    {
        strcat(command, "0%");
    }
    // I have set the range of iframes to be 2 minutes before the given time to 1 minute after the given time
    sprintf(end, "%" PRIu64, *start_time_sec + 60);
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
    while (fgets(line, sizeof(line), process) != NULL)
    {
        // remove \n and trailing zeroes
        if ((strlen(line) > 0) && (line[strlen(line) - 1] == '\n'))
            line[strlen(line) - 1] = '\0';
        // store Iframes into array
        iframes[*counter] = atof(line);
        //*counter++ doesn't work supposedly
        *counter = *counter + 1;
    }
    // reallocate the iframes to save memory
    iframes = (long double *)realloc(iframes, *counter * sizeof(long double));
    pclose(process);
    printf("Here are the following iframe positions in seconds: \n");
    for (uint8_t i = 0; i < *counter; i++)
    {
        printf("%Lf \n", iframes[i]);
    }
}

int create_video(long double *iframe, char *file_directory, char *end_time, uint8_t *i)
{
    char command[500] = "ffmpeg -i '";
    strcat(command, file_directory);
    strcat(command, "'");
    strcat(command, " -ss ");
    char start_time[50];
    char num[5];
    sprintf(start_time, "%Lf", *iframe);
    strcat(command, start_time);
    strcat(command, " -to ");
    strcat(command, end_time);
    strcat(command, " -c copy output");
    sprintf(num, "%" PRIu8, *i);
    strcat(command, num);
    strcat(command, ".mp4");
    printf("%s", command);
    char line[200];
    size_t chunks;
    FILE *process;
    process = popen(command, "r");
    if (process == NULL)
    {
        printf("Command failed to execute\n");
        return 1;
    }
    pclose(process);
}
int main() // gets all user input and executes the other functions
{
    char file_directory[100]; // I'm assuming your file directory is less than 100 characters long
    char *destination_path = NULL;
    char start_time[10] = {'\0'};
    char end_time[10] = {'\0'};
    uint64_t start_time_sec = 0.0;
    uint8_t counter = 0;
    long double *iframes = malloc(10000 * sizeof(long double));
    if (iframes == NULL)
    {
        printf(" Memory allocation failed. Exiting the program .\n");
        return 1;
    }
    get_input(file_directory, destination_path, start_time, end_time);
    extracting_time(start_time, &start_time_sec);
    get_iframes(&start_time_sec, iframes, file_directory, &counter);
    for (uint8_t i = 0; i < counter; i++)
    {
        create_video(&iframes[i], file_directory, end_time, &i);
    }
}

/*
 * Dakota Kallas & Jack Parker
 *
 * CS441/541: Project 3
 *
 */
#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>

/* For fork, exec, sleep */
#include <sys/types.h>
#include <unistd.h>
/* For waitpid */
#include <sys/wait.h>

/******************************
 * Defines
 ******************************/
#define TRUE 1
#define FALSE 0

#define MAX_COMMAND_LINE 1024

#define PROMPT ("mysh$ ")

/******************************
 * Structures
 ******************************/
/*
 * A job struct.  Feel free to change as needed.
 */
struct job_t
{
    char *full_command;
    int argc;
    char **argv;
    int is_background;
    char *binary;
    int pid;
};
typedef struct job_t job_t;

struct job_node
{
    int pid;
    bool running;
    char *full_command;
    struct job_node *next;
    int job_num;
};
typedef struct job_node job_node;

/******************************
 * Global Variables
 ******************************/

/*
 * Interactive or batch mode
 */
int is_batch;
FILE **input_files;

/*
 * Counts
 */
int total_jobs_display_ctr = 0;
int total_jobs = 0;
int total_jobs_bg = 0;
int total_history = 0;
int num_files;

/*
 * Job Lists
 */
job_node *bg_jobs_head = NULL;
job_node *bg_jobs_tail = NULL;
job_node *all_jobs_head = NULL;
job_node *all_jobs_tail = NULL;

/*
 * File I/O
 */
bool file_input = false;
int input_file;
bool file_output = false;
int output_file;

/*
 * Debugging mode
 */
int is_debug = FALSE;

/******************************
 * Function declarations
 ******************************/
/*
 * Parse command line arguments passed to myshell upon startup.
 *
 * Parameters:
 *  argc : Number of command line arguments
 *  argv : Array of pointers to strings
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int parse_args_main(int argc, char **argv);

/*
 * Main routine for batch mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int batch_mode(void);

/*
 * Main routine for interactive mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int interactive_mode(void);

/*
 * Launch a job
 *
 * Parameters:
 *   loc_job : job to execute
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int launch_job(job_t *loc_job);

/*
 * Built-in 'exit' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_exit(void);

/*
 * Built-in 'jobs' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_jobs(void);

/*
 * Built-in 'history' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_history(void);

/*
 * Built-in 'wait' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_wait(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   None (use default behavior)
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   Job id
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg_num(int job_num);

/**
 * @brief Prints out the final counts of jobs before exiting
 *
 */
void display_counts(void);

/**
 * @brief Given an array of strings, concatenate all strings together with the joint separating them into one long string
 *
 * @param size The size of the array
 * @param array The array of strings
 * @param joint What character should separate each string
 *
 * @return char* That represents all strings joined together
 */
char *concatenate(size_t size, char *array[size], const char *joint);

/**
 * @brief Parse a given input and designate jobs for it
 *
 * @param input User given input used to be entered into the shell
 */
void parse_job(char *input);

/**
 * @brief Add a given job to the larger list of jobs
 *
 * @param loc_job Job to be added to respective lists
 */
void add_job_to_lists(job_t *loc_job);

/**
 * @brief Remove all finished jobs from the list of current jobs
 *
 */
void removeFinishedJobs();

#endif /* MYSHELL_H */

/*
 * Dakota Kallas & Jack Parker
 *
 * CS441/541: Project 3
 *
 */
#include "mysh.h"

int main(int argc, char *argv[])
{
    int ret;

    /*
     * Parse Command line arguments to check if this is an interactive or batch
     * mode run.
     */
    if (0 != (ret = parse_args_main(argc, argv)))
    {
        fprintf(stderr, "Error: Invalid command line!\n");
        return -1;
    }

    /*
     * If in batch mode then process all batch files
     */
    if (TRUE == is_batch)
    {
        if (TRUE == is_debug)
        {
            printf("Batch Mode!\n");
        }

        if (0 != (ret = batch_mode()))
        {
            fprintf(stderr, "Error: Batch mode returned a failure!\n");
        }
    }
    /*
     * Otherwise proceed in interactive mode
     */
    else if (FALSE == is_batch)
    {
        if (TRUE == is_debug)
        {
            printf("Interactive Mode!\n");
        }

        if (0 != (ret = interactive_mode()))
        {
            fprintf(stderr, "Error: Interactive mode returned a failure!\n");
        }
    }
    /*
     * This should never happen, but otherwise unknown mode
     */
    else
    {
        fprintf(stderr, "Error: Unknown execution mode!\n");
        return -1;
    }

    /*
     * Display counts
     */
    display_counts();

    /*
     * Cleanup
     */

    return 0;
}

void display_counts(void)
{
    printf("-------------------------------\n");
    printf("Total number of jobs               = %d\n", total_jobs);
    printf("Total number of jobs in history    = %d\n", total_history);
    printf("Total number of jobs in background = %d\n", total_jobs_bg);
}

int parse_args_main(int argc, char **argv)
{
    int i;

    if (argc <= 1) // no args, we're in interactive mode.
    {
        is_batch = false;
    }
    else // args, we're in batch mode.
    {
        is_batch = true;

        input_files = malloc(sizeof(FILE *) * (argc - 1));
        for (i = 1; i < argc; i++)
        {
            num_files++;
            input_files[i - 1] = fopen(argv[i], "r");
            if (input_files[i - 1] == NULL)
            {
                fprintf(stderr, "\nCould not open file: %s\n", argv[i]);
                num_files--;
                argc--;
                i--;
                continue;
            }
        }
    }

    return 0;
}

int batch_mode(void)
{

    int i;
    char input[MAX_COMMAND_LINE];
    char *line;

    for (i = 0; i < num_files; i++) // for each file...
    {
        line = fgets(input, MAX_COMMAND_LINE, input_files[i]); // read one line at a time
        while (line != NULL || !feof(input_files[i]))
        {
            parse_job(line);
            line = fgets(input, MAX_COMMAND_LINE, input_files[i]);
        }
        fclose(input_files[i]); // close the file
    }

    builtin_exit();
    return 0;
}

int interactive_mode(void)
{
    char input[MAX_COMMAND_LINE];

    while (1)
    {
        printf(PROMPT);
        // read a line of input
        if (fgets(input, MAX_COMMAND_LINE, stdin) == NULL)
        {
            printf("\n");
            builtin_exit();
        }
        if (input[0] == '\n')
        {
            continue; // user pressed Enter to exit
        }

        parse_job(input);
    }
    return 0;
}

char *concatenate(size_t size, char *array[size], const char *joint)
{
    size_t jlen, lens[size];
    size_t i, total_size = (size - 1) * (jlen = strlen(joint)) + 1;
    char *result, *p;

    for (i = 0; i < size; ++i)
    {
        total_size += (lens[i] = strlen(array[i]));
    }
    p = result = malloc(total_size);
    for (i = 0; i < size; ++i)
    {
        memcpy(p, array[i], lens[i]);
        p += lens[i];
        if (i < size - 1)
        {
            memcpy(p, joint, jlen);
            p += jlen;
        }
    }
    *p = '\0';
    return result;
}

void parse_job(char *input)
{
    input[strcspn(input, "\n")] = '\0';

    int argc = 0;
    bool first = true;
    char *word;
    bool in_opened = false;
    bool out_opened = false;
    bool more_args = true;

    job_t *current_job = (job_t *)malloc(sizeof(job_t));
    // allocate memory for argv
    current_job->argv = malloc(MAX_COMMAND_LINE * sizeof(char *));
    // allocate memory for full_command and copy input to it
    current_job->full_command = malloc(strlen(input) + 1);

    word = strtok(input, " "); // get the first word
    while (word != NULL)
    {
        // check if this is the end of the current job
        if (strcmp(word, "&") == 0)
        {
            // finish setup of current job
            current_job->full_command = concatenate(argc, current_job->argv, " ");
            current_job->is_background = true;
            total_jobs_bg++;
            launch_job(current_job);

            // cleanup current job & reinitialize for further use
            for (int i = 0; i < current_job->argc; i++)
            {
                free(current_job->argv[i]);
            }
            free(current_job->binary);
            free(current_job);
            current_job = (job_t *)malloc(sizeof(job_t));
            current_job->argv = malloc(MAX_COMMAND_LINE * sizeof(char *));
            current_job->full_command = malloc(strlen(input) + 1);
            argc = 0;
            first = true;
        }
        else if (strcmp(word, ";") == 0)
        {
            // finish setup of current job
            current_job->full_command = concatenate(argc, current_job->argv, " ");
            current_job->argc = argc;
            launch_job(current_job);

            // cleanup current job & reinitialize for further use
            for (int i = 0; i < current_job->argc; i++)
            {
                free(current_job->argv[i]);
            }
            free(current_job->binary);
            free(current_job);
            current_job = (job_t *)malloc(sizeof(job_t));
            current_job->argv = malloc(MAX_COMMAND_LINE * sizeof(char *));
            current_job->full_command = malloc(strlen(input) + 1);
            argc = 0;
            first = true;
        }
        else if (strcmp(word, ">") == 0)
        { // check for output redirection
            file_output = true;
        }
        else if (strcmp(word, "<") == 0)
        { // check for input redirection
            file_input = true;
        }
        else
        {
            if (file_output && !out_opened)
            { // open the file for outputting to
                out_opened = true;
                more_args = false;
                output_file = open(word, O_CREAT | O_TRUNC | O_RDWR, 0644);
            }
            if (file_input && !in_opened)
            { // open the file for reading from
                in_opened = true;
                more_args = false;
                input_file = open(word, O_RDONLY);
            }

            if (first)
            {
                current_job->binary = malloc(strlen(word) + 1);
                strcpy(current_job->binary, word);
                first = false;
            }

            if (more_args)
            {
                // allocate memory for the argument string and copy word to it
                current_job->argv[argc] = malloc(strlen(word) + 1);
                strcpy(current_job->argv[argc], word);
                argc++;
            }
            
        }

        word = strtok(NULL, " "); // get the next word
    }
    // finish setup of current job
    current_job->argc = argc;
    current_job->full_command = concatenate(argc, current_job->argv, " ");

    if (argc > 0 && strcmp(current_job->argv[argc - 1], "&") == 0)
    {
        current_job->is_background = true;
        current_job->argv[argc - 1] = NULL;
    }
    if (argc > 0)
    {
        launch_job(current_job);

        // free memory allocated for full_command and argv
        free(current_job->full_command);
        free(current_job->binary);
        for (int i = 0; i < current_job->argc; i++)
        {
            free(current_job->argv[i]);
        }

        // free memory allocated for current_job
        free(current_job);
    }
}

/*
 * You will want one or more helper functions for parsing the commands
 * and then call the following functions to execute them
 */

int launch_job(job_t *loc_job)
{

    /*
     * Display the job
     */
    if (is_debug == TRUE)
    {
        printf("\n/-----------------------------/\n");
        printf("COMMAND: %s\n", loc_job->full_command);
        printf(" BINARY: %s\n", loc_job->binary);
        printf("   ARGC: %d\n", loc_job->argc);
        printf("BCKGRND? %d\n", loc_job->is_background);
        printf("/-----------------------------/\n\n");
    }

    /*
     * Launch the job in either the foreground or background
     */

    if (strcmp(loc_job->binary, "exit") == 0)
    {
        // free memory allocated for full_command and argv
        free(loc_job->full_command);
        for (int i = 0; i < loc_job->argc; i++)
        {
            free(loc_job->argv[i]);
        }

        // free memory allocated for loc_job
        free(loc_job);

        total_history++;
        builtin_exit();
        return 0;
    }

    // call proper builtins if necessary
    if (strcmp(loc_job->binary, "jobs") == 0)
    {
        // add jobs to lists for jobs and history builtins
        add_job_to_lists(loc_job);
        total_history++;
        builtin_jobs();
    }
    else if (strcmp(loc_job->binary, "history") == 0)
    {
        // add jobs to lists for jobs and history builtins
        add_job_to_lists(loc_job);
        total_history++;
        builtin_history();
    }
    else if (strcmp(loc_job->binary, "wait") == 0)
    {
        // add jobs to lists for jobs and history builtins
        add_job_to_lists(loc_job);
        total_history++;
        builtin_wait();
    }
    else if ((strcmp(loc_job->binary, "fg") == 0) && (loc_job->argc > 1))
    {
        // add jobs to lists for jobs and history builtins
        add_job_to_lists(loc_job);
        total_history++;
        builtin_fg_num(strtol(loc_job->argv[1], NULL, 10));
    }
    else if (strcmp(loc_job->binary, "fg") == 0)
    {
        // add jobs to lists for jobs and history builtins
        add_job_to_lists(loc_job);
        total_history++;
        builtin_fg();
    }
    else
    {
        total_jobs++;
        total_history++;
        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Error: Fork failed!\n");
            return -1;
        }
        else if (pid == 0) // child
        {
            // add jobs to lists for jobs and history builtins
            if (file_output)
            { // redirect output
                file_output = false;
                dup2(output_file, STDOUT_FILENO); // replace stdout with desired output file
                close(output_file);               // close??? (idk it works)
            }
            if (file_input)
            { // redirect input
                file_input = false;
                dup2(input_file, STDIN_FILENO); // replace stdin with desired input file
                close(input_file);
            }

            execvp(loc_job->binary, loc_job->argv);
            fprintf(stderr, "Error: Invalid command.\n");
            exit(-1);
        }
        else // parent
        {
            loc_job->pid = pid;
            // add jobs to lists for jobs and history builtins
            add_job_to_lists(loc_job);
            if (file_output)
            {
                close(output_file);
                file_output = false;
            }
            if (file_input)
            {
                close(input_file);
                file_input = false;
            }

            if (!loc_job->is_background)
            {
                // Wait for the child process to finish if running in foreground
                waitpid(pid, NULL, 0);
            }
        }
    }

    /*
     * Some accounting
     */

    return 0;
}

void add_job_to_lists(job_t *loc_job)
{

    job_node *all_job_node;
    job_node *bg_job_node;
    char *job_command;

    // save the job command into all_jobs_head linked list for bookkeeping
    if (!all_jobs_head)
    { // first node

        // malloc space for node and command
        all_job_node = (job_node *)malloc(sizeof(job_node));
        job_command = malloc(strlen(loc_job->full_command) + 1);

        // fill new node with data
        strcpy(job_command, loc_job->full_command);
        all_job_node->full_command = job_command;
        all_job_node->running = true;

        // fill new node with data
        all_jobs_head = all_job_node;
        all_jobs_tail = all_job_node;
    }
    else
    { // not first node

        // malloc space for node and command
        all_job_node = (job_node *)malloc(sizeof(job_node));
        job_command = malloc(strlen(loc_job->full_command) + 1);

        // fill new node with data
        strcpy(job_command, loc_job->full_command);
        all_job_node->full_command = job_command;
        all_job_node->running = true;

        // fill new node with data
        all_jobs_tail->next = all_job_node;
        all_jobs_tail = all_job_node;
    }

    if (loc_job->is_background)
    { // do the same with the background_jobs link list if its a background job
        if (!bg_jobs_head)
        { // first node

            // malloc space for node and command
            bg_job_node = (job_node *)malloc(sizeof(job_node));
            job_command = malloc(strlen(loc_job->full_command) + 1);

            // fill new node with data
            strcpy(job_command, loc_job->full_command);
            bg_job_node->full_command = job_command;
            bg_job_node->running = true;
            bg_job_node->pid = loc_job->pid;
            bg_job_node->job_num = 1;

            // fill new node with data
            bg_jobs_head = bg_job_node;
            bg_jobs_tail = bg_job_node;
        }
        else
        { // not first node

            // malloc space for node and command
            bg_job_node = (job_node *)malloc(sizeof(job_node));
            job_command = malloc(strlen(loc_job->full_command) + 1);

            // fill new node with data
            strcpy(job_command, loc_job->full_command);
            bg_job_node->full_command = job_command;
            bg_job_node->running = true;
            bg_job_node->pid = loc_job->pid;
            bg_job_node->job_num = bg_jobs_tail->job_num + 1;

            // fill new node with data
            bg_jobs_tail->next = bg_job_node;
            bg_jobs_tail = bg_job_node;
        }
        if (bg_job_node->pid > 0)
        {
            printf("[%d] %d\n", bg_job_node->job_num, loc_job->pid);
        }
    }
}

int builtin_exit(void)
{
    job_node *curr_job_node = bg_jobs_head;
    int jobs_waiting = 0;

    while (curr_job_node != NULL)
    {
        int rtn_pid = waitpid(curr_job_node->pid, NULL, WNOHANG);

        if (rtn_pid > 0)
        {
            curr_job_node->running = false;
        }
        else
        {
            jobs_waiting++;
        }

        curr_job_node = curr_job_node->next;
    }

    if (jobs_waiting > 0)
    {
        printf("Still waiting on %d job(s)...\n", jobs_waiting);
    }

    removeFinishedJobs();

    if (builtin_wait() == 0)
    {
        display_counts();
        exit(1);
    }
    return 0;
}

int builtin_jobs(void)
{

    job_node *curr_job_node = bg_jobs_head;

    // print all jobs in the list
    while (curr_job_node != NULL)
    {
        int rtn_pid = waitpid(curr_job_node->pid, NULL, WNOHANG);
        char *state = "Running";
        char *identifier = "+";

        if (rtn_pid < 0)
        {
            if (!curr_job_node->running)
            {
                state = "Done";
                identifier = "";
            }
        }
        else if (rtn_pid > 0)
        {
            curr_job_node->running = false;
            state = "Done";
            identifier = "";
        }

        printf("[%d]%s\t%s\t%s\n", curr_job_node->job_num, identifier, state, curr_job_node->full_command);
        curr_job_node = curr_job_node->next;
    }

    // delete jobs that are finsished from the list
    removeFinishedJobs();

    return 0;
}

void removeFinishedJobs()
{
    job_node *curr_job_node = bg_jobs_head;
    job_node *prev_node;

    while (curr_job_node != NULL)
    {
        if (!curr_job_node->running)
        {
            if ((curr_job_node == bg_jobs_head) && (curr_job_node == bg_jobs_tail))
            { // only node needs removal
                bg_jobs_head = NULL;
                bg_jobs_tail = NULL;
            }
            else if (curr_job_node == bg_jobs_head)
            { // head nodes needs removal
                bg_jobs_head = curr_job_node->next;
            }
            else if (curr_job_node == bg_jobs_tail)
            { // tail node needs removal
                bg_jobs_tail = prev_node;
                bg_jobs_tail->next = NULL;
            }
            else
            { // mid-list node needs removal
                prev_node->next = curr_job_node->next;
            }
        }
        prev_node = curr_job_node;
        curr_job_node = curr_job_node->next;
    }
}

int builtin_history(void)
{

    job_node *curr_job_node = all_jobs_head;
    int i = 1;

    while (curr_job_node != NULL)
    {
        printf("   %d\t%s\n", i, curr_job_node->full_command);
        i++;
        curr_job_node = curr_job_node->next;
    }

    return 0;
}

int builtin_wait(void)
{
    job_node *curr_job_node = bg_jobs_head;

    while (curr_job_node != NULL)
    {
        int rtn_pid = waitpid(curr_job_node->pid, NULL, 0);

        if (rtn_pid < 0)
        {
            return -1;
        }
        else if (rtn_pid > 0)
        {
            curr_job_node->running = false;
        }

        curr_job_node = curr_job_node->next;
    }

    return 0;
}

int builtin_fg(void)
{
    if (bg_jobs_tail != NULL)
    {
        int rtn_pid = waitpid(bg_jobs_tail->pid, NULL, WNOHANG);

        if (rtn_pid > 0)
        {
            bg_jobs_tail->running = false;
            printf("[%d]+\tDone\t%s\n", bg_jobs_tail->job_num, bg_jobs_tail->full_command);
        }
        else
        {
            printf("%s\n", bg_jobs_tail->full_command);
            int rtn_pid = waitpid(bg_jobs_tail->pid, NULL, 0);

            if (rtn_pid < 0)
            {
                return -1;
            }
            else
            {
                bg_jobs_tail->running = false;
            }
        }
    }

    // delete jobs that are finsished from the list
    removeFinishedJobs();

    return 0;
}

int builtin_fg_num(int job_num)
{
    job_node *curr_job_node = bg_jobs_head;

    // print all jobs in the list
    while (curr_job_node != NULL)
    {
        if (curr_job_node->job_num == job_num)
        {
            int rtn_pid = waitpid(curr_job_node->pid, NULL, WNOHANG);

            if (rtn_pid > 0)
            {
                curr_job_node->running = false;
                printf("[%d]+\tDone\t%s\n", curr_job_node->job_num, curr_job_node->full_command);
            }
            else
            {
                printf("%s\n", curr_job_node->full_command);
                int rtn_pid = waitpid(curr_job_node->pid, NULL, 0);

                if (rtn_pid < 0)
                {
                    return -1;
                }
                else
                {
                    curr_job_node->running = false;
                }
            }
        }

        curr_job_node = curr_job_node->next;
    }

    // delete jobs that are finsished from the list
    removeFinishedJobs();

    return 0;
}

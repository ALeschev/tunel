#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>

#define trace_err(fmt, ...) \
        do { \
            fprintf(stderr, "[%s] %s:%d:%s(): Error: "fmt"\n", get_time(), \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
            exit(1);\
        } while(0)

#define trace_warn(fmt, ...) \
        do { \
            fprintf(stderr, "[%s] %s:%d:%s(): Warn: "fmt"\n", get_time(), \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
        } while(0)

#ifdef DEBUG
#define trace_info(fmt, ...) \
        do { \
            fprintf(stdout, "[%s] %s:%d:%s(): Info: "fmt"\n", get_time(), \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
        } while(0)
#else
#define trace_info(fmt, ...) do{}while(0)
#endif

#define MAX_NODES 250
#define MAX_INPUT_STR 16
#define ONE_SEC 1000000

enum node_state { INITED, LAUNCHED, SEARCHING, COMPLETED };
enum file_run_mode { C_lang, BASH };

typedef struct
{
    pthread_t node_id;
    int state;
    int activated;
    char data[MAX_INPUT_STR];
} node_t;

typedef struct
{
    node_t nodes[MAX_NODES];
    int cur_max;
} node_data_t;

char *get_time(void)
{
    static char cBuffer[100];
    time_t zaman;
    struct tm *ltime;
    static struct timeval _t;
    static struct timezone tz;

    time(&zaman);
    ltime = (struct tm *) localtime(&zaman);
    gettimeofday(&_t, &tz);

    strftime(cBuffer,40, "%H:%M:%S",ltime);
    sprintf(cBuffer, "%s.%d", cBuffer,(int)_t.tv_usec);

    return cBuffer;
}

char *node_state_str(int state)
{
    switch (state)
    {
        case INITED: return "Inited";
        case LAUNCHED: return "Launched";
        case SEARCHING: return "Searching";
        case COMPLETED: return "Completed";
    }

    return "Undefined";
}

void node_set_state (node_t *node, int state)
{
    if (!node)
    {
        trace_warn("Failed to set node state. Node is nil");
        return;
    }

    if (node->state == state)
        return;

    trace_info("Node <%p> state changed: %s -> %s", &node->node_id,
               node_state_str(node->state), node_state_str(state));

    node->state = state;

    switch (state)
    {
        case INITED:
            node->activated = 0;
        break;
        case LAUNCHED:
            node->activated = 1;
        break;
        case SEARCHING:
            node->activated = 0;
        break;
        case COMPLETED:
        break;
    }
}

node_t *node_get_free(node_data_t *node_data)
{
    int i;
    node_t *p_node;

    if (!node_data)
        return NULL;

    for (i = 0; i < node_data->cur_max; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->state == INITED)
            return p_node;
    }

    for (i = node_data->cur_max; i < MAX_NODES; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->state == INITED)
        {
            node_data->cur_max++;
            return p_node;
        }
    }

    return NULL;
}

void node_set_free(node_data_t *node_data, pthread_t node_id)
{
    int i;
    node_t *p_node;

    if (!node_data)
        return;

    for (i = 0; i < node_data->cur_max; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->node_id == node_id)
        {
            node_set_state(p_node, INITED);
            return;
        }
    }

    for (i = node_data->cur_max; i < MAX_NODES; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->node_id == node_id)
        {
            node_set_state(p_node, INITED);

            if (node_data->cur_max > 0)
                node_data->cur_max--;

            return;
        }
    }
}

void node_clear_all(node_data_t *node_data, int force)
{
    int i;
    node_t *p_node;

    for (i = 0; i < MAX_NODES; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->state != INITED)
        {
            if (force)
                node_set_state(p_node, INITED);

            pthread_join(p_node->node_id, NULL);

            if (node_data->cur_max > 0)
                node_data->cur_max--;
        }
    }
}

int compile_file(node_t *node, char *filename)
{
    int status;
    char command[32 + MAX_INPUT_STR] = {0};

    if (!filename)
        return 1;

    sprintf (command, "gcc %s.c -o %s_e 2> %s_compile.out",
            filename, filename, filename);

    trace_info("Node <%p>. Try exec: %s", &node->node_id, command);

    status = system(command);

    trace_info("Node <%p>. %s.c: Compile status %s", &node->node_id,
                filename, status? "NOK":"OK");

    return status;
}

void run_file (node_t *node, char *filename, int mode)
{
    int status;
    char command[16 + MAX_INPUT_STR] = {0};

    if (!filename)
        return;

    switch(mode)
    {
        case C_lang:
            sprintf(command, "./%s_e 1> %s_run.out", filename, filename);
        break;

        case BASH:
            sprintf(command, "bash ./%s.sh > %s_run.out", filename, filename);
        break;

        default:
            trace_err("Node <%p>. %s_e: Unknown run-mode", &node->node_id, filename);
    }

    if (strlen(command))
    {
        status = system(command);
        trace_info("Node <%p>. %s_e: Run status: %s", &node->node_id,
                        filename, status? "NOK":"OK");
    }
}

int scan_file(node_t *node, char *filename, char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    int have_source_file = 0;

    dir = opendir(dirname);
    if (dir == NULL) 
    {
        trace_err("Failed to open '%s'", dirname);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL && node->activated)
    {
        if (entry->d_type != DT_REG)
            continue;

        if (strlen(entry->d_name) != (strlen(filename) + 2))
            continue;

        if (!strncmp(entry->d_name, filename, strlen(filename)))
        {
            if (!strcmp(&entry->d_name[strlen(entry->d_name) - 2], ".c"))
            {
                trace_info("Node <%p>. Found file %s - is C source file",
                            &node->node_id, filename);

                have_source_file = 1;
            }
        }
    }

    closedir(dir);

    return have_source_file;
}

int scan_dir(node_t *node, char *filename, char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    int have_dir = 0;

    dir = opendir(dirname);
    if (dir == NULL) 
    {
        trace_err("Failed to open '%s'", dirname);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL && node->activated)
    {
        if (entry->d_type == DT_DIR)
        {
            if (!strcmp(entry->d_name, filename))
            {
                have_dir= 1;
            }
        }
    }

    closedir(dir);

    return have_dir;
}

int scan_bash(node_t *node, char *filename, char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    int have_source_file = 0;

    dir = opendir(dirname);
    if (dir == NULL) 
    {
        trace_err("Failed to open '%s'", dirname);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL && node->activated)
    {
        if (entry->d_type != DT_REG)
            continue;

        if (strlen(entry->d_name) != (strlen(filename) + 3))
            continue;

        if (!strncmp(entry->d_name, filename, strlen(filename)))
        {
            if (!strcmp(&entry->d_name[strlen(entry->d_name) - 3], ".sh"))
            {
                trace_info("Node <%p>. Found file %s - is Bash source file",
                            &node->node_id, filename);

                have_source_file = 1;
            }
        }
    }

    closedir(dir);

    return have_source_file;
}


int scandirectory(node_t *node, char *filename, char *dirname)
{
    int file_entry = 0, dir_entry = 0, bash_entry = 0;
    char buff[128] = {0};

    sprintf (buff, "echo \"[%s] %s:%d:%s(): Info: Node <%p>. "
                   "Filename %s. Start at '$PWD'\"", get_time(),
                   __FILE__, __LINE__, __func__,
                   &node->node_id, filename);
    system(buff);

    file_entry = scan_file(node, filename, dirname);
    if (file_entry)
    {
        if (compile_file(node, filename) == 0)
            run_file(node, filename, C_lang);
    }

    dir_entry = scan_dir(node, filename, dirname);
    if (dir_entry)
    {
        sprintf (buff, "echo \"[%s] %s:%d:%s(): Info: Node <%p>. "
                       "Found dir '%s' in '$PWD'\"", get_time(),
                   __FILE__, __LINE__, __func__,
                   &node->node_id, filename);
        system(buff);

        chdir(filename);
        // trace_info("Node <%p>. Found dir %s", &node->node_id, filename);
        scandirectory(node, filename, "./");
    }

    if (!file_entry && !dir_entry)
    {
        bash_entry = scan_bash(node, filename, dirname);
        if (bash_entry)
        {
            trace_info("Node <%p>. Try run bash '%s'", &node->node_id, filename);
            run_file(node, filename, BASH);
        } else {
            trace_info("Node <%p>. Bash script with name '%s' not found",
                       &node->node_id, filename);
        }

    }

    return 1;
}

void *node_thread(void *arg)
{
    node_t *node = (node_t *)arg;

    while (node->activated)
    {
        chdir("./dir1/");
        if (scandirectory(node, node->data, "./"))
            break;
    }

    trace_info("Node <%p>: completed", &node->node_id);

    node_set_state(node, INITED);

    return NULL;
}

void *worker_thread(void *arg)
{
    FILE *file_fd = NULL;
    node_data_t node_data;
    node_t *p_node;
    char *data_file = NULL;
    char input_data[MAX_INPUT_STR] = {0};
    int rand_time = 0;

    data_file = (char *)arg;
    if (!data_file)
    {
        trace_err("Worker thread start failed. "
                  "Failed to get data file.");
    }

    file_fd = fopen(data_file, "r");
    if (!file_fd)
    {
        trace_err("Worker thread start failed. "
                  "Failed to open data file.");
    }

    memset(&node_data, 0, sizeof (node_data));

    srand(time(NULL));

    while (1)
    {
        rand_time = rand() % ONE_SEC;

        usleep(rand_time);

        if (fscanf(file_fd, "%s", input_data) == EOF)
        {
            trace_info("Detect end of file. Deactivate threads");
            break;
        }

        trace_info("command after %.3f sec: %s",
                   (float)rand_time/ONE_SEC, input_data);

        p_node = node_get_free(&node_data);
        if (!p_node)
        {
            trace_err("Failed to get free node. '%s' skipped", input_data);
            continue;
        }

        memcpy (p_node->data, input_data, MAX_INPUT_STR);

        node_set_state(p_node, LAUNCHED);

        pthread_create(&p_node->node_id, NULL, node_thread, (void *)p_node);
    }

    node_clear_all(&node_data, 0/*wait/force*/);

    fclose(file_fd);

    return NULL;
}

int main()
{
    pthread_t worker_th;
    char data_file[] = "data_file";

    trace_info("Work crew started");

    pthread_create (&worker_th, NULL, worker_thread, (void *)data_file);

    pthread_join (worker_th, NULL);

    trace_info("Work crew finished");

    return 0;
}

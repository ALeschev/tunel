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
#include <signal.h>

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
#define START_DIR "./dir1"

enum file_run_mode { C_lang, BASH };
enum worker_state { STOPPED, STARTED, HARD_STOP, SOFT_STOP };
enum node_state { INITED, LAUNCHED, SEARCHING, COMPLETED };

typedef struct
{
    pthread_t node_th;
    int node_id;
    int state;
    int activated;
    char data[MAX_INPUT_STR];
    char dir[256];
} node_t;

typedef struct
{
    node_t nodes[MAX_NODES];
    int cur_max;
} node_data_t;

static int worker_state = STOPPED;

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

char *worker_state_str(int state)
{
    switch (state)
    {
        case STARTED: return "Started";
        case STOPPED: return "Stopped";
        case HARD_STOP: return "Hard stop";
        case SOFT_STOP: return "Soft stop";
    }

    return "Undefined";
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

void worker_set_state (int state)
{
    if (worker_state == state)
        return;

    trace_info("Worker state changed: %s -> %s",
                worker_state_str(worker_state),
                worker_state_str(state));

    worker_state = state;
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

    trace_info("Node <%d> state changed: %s -> %s", node->node_id,
               node_state_str(node->state), node_state_str(state));

    switch (node->state)
    {
        case INITED:
            switch (state)
            {
                case INITED:
                break;
                case LAUNCHED:
                    node->activated = 1;
                break;
            }
        break;
        case LAUNCHED:
            switch (state)
            {
                case INITED:
                    node->activated = 0;
                break;
                case SEARCHING:
                    /*not change*/
                break;
            }
        break;
        case SEARCHING:
            switch (state)
            {
                case INITED:
                    node->activated = 0;
                break;
                case COMPLETED:
                    node->activated = 0;
                break;
            }
        break;
        case COMPLETED:
            switch (state)
            {
                case INITED:
                    node->activated = 0;
                break;
            }
        break;
    }

        node->state = state;
}

node_t *node_get_free(node_data_t *node_data)
{
    int i;
    node_t *p_node;

    if (!node_data)
        return NULL;

    for (i = node_data->cur_max; i < MAX_NODES; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->state == INITED)
        {
            p_node->node_id = i;
            node_data->cur_max++;
            return p_node;
        }
    }

    for (i = 0; i < node_data->cur_max; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->state == INITED)
        {
            p_node->node_id = i;
            return p_node;
        }
    }

    return NULL;
}

void node_set_free(node_data_t *node_data, int node_id)
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
            p_node->node_id = -1;
            node_set_state(p_node, INITED);
            return;
        }
    }

    for (i = node_data->cur_max; i < MAX_NODES; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->node_id == node_id)
        {
            p_node->node_id = -1;
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

    trace_info("Deactivate threads <%s>", force? "HARD":"SOFT");

    for (i = 0; i < MAX_NODES; i++)
    {
        p_node = &node_data->nodes[i];

        if (p_node->state != INITED)
        {
            if (force)
            {
                node_set_state(p_node, INITED);
                pthread_cancel(p_node->node_th);
            }

            pthread_join(p_node->node_th, NULL);

            if (!force)
                node_set_state(p_node, INITED);

            p_node->node_id = -1;

            if (node_data->cur_max > 0)
                node_data->cur_max--;
        }
    }
}

int compile_file(node_t *node, char *filename, char *dirname)
{
    int status = 1;
    char command[64 + MAX_INPUT_STR] = {0};

    if (!filename || !dirname)
        return 1;

    sprintf (command, "gcc %s/%s.c -o %s/%s_e 2> %s/%s_compile_out",
            dirname, filename, dirname, filename, dirname, filename);

    trace_info("Node <%d>. Try exec: %s", node->node_id, command);

    status = system(command);

    trace_info("Node <%d>. %s.c: Compile status %s", node->node_id,
                filename, status? "NOK":"OK");

    return status;
}

void run_file (node_t *node, char *filename, char *dirname, int mode)
{
    int status;
    char command[64 + MAX_INPUT_STR] = {0};

    if (!filename || !dirname)
        return;

    switch(mode)
    {
        case C_lang:
            sprintf(command, "%s/%s_e %d 1> %s/%s_run_out",
                    dirname, filename, node->node_id, dirname, filename);
        break;

        case BASH:
            sprintf(command, "bash %s/%s.sh %d > %s/%s_run_out",
                    dirname, filename, node->node_id, dirname, filename);
        break;

        default:
            trace_err("Node <%d>. %s_e: Unknown run-mode", node->node_id, filename);
    }

    if (strlen(command))
    {
        status = system(command);
        trace_info("Node <%d>. %s_e: Run status: %s", node->node_id,
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
                have_source_file = 1;
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
                have_dir= 1;
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
                have_source_file = 1;
        }
    }

    closedir(dir);

    return have_source_file;
}


int scandirectory(node_t *node, char *filename, char *dirname)
{
    int file_entry = 0, dir_entry = 0, bash_entry = 0;
    char buff[128] = {0};

    sprintf (buff, "echo \"[%s] %s:%d:%s(): Info: Node <%d>. "
                   "Filename %s. Start at '%s'\"", get_time(),
                   __FILE__, __LINE__, __func__,
                   node->node_id, filename, dirname);
    system(buff);

    file_entry = scan_file(node, filename, dirname);
    if (file_entry)
    {
        trace_info("Node <%d>. Found file %s in %s. Its C source file",
                            node->node_id, filename, node->dir);

        if (compile_file(node, filename, dirname) == 0)
        {
            run_file(node, filename, dirname, C_lang);
        }
    }

    dir_entry = scan_dir(node, filename, dirname);
    if (dir_entry)
    {
        // node->dir[strlen(node->dir)] = '/';
        sprintf (node->dir, "%s/%s", node->dir, filename);
        // strcat (node->dir, filename);

        sprintf (buff, "echo \"[%s] %s:%d:%s(): Info: Node <%d>. "
                       "Found dir '%s' in '$PWD/%s'\"", get_time(),
                       __FILE__, __LINE__, __func__,
                       node->node_id, filename, node->dir);
        system(buff);

        // chdir(filename);
        // trace_info("Node <%d>. Found dir %s", node->node_id, filename);
        scandirectory(node, filename, node->dir);
    }

    if (!file_entry && !dir_entry)
    {
        bash_entry = scan_bash(node, filename, dirname);
        if (bash_entry)
        {
            trace_info("Node <%d>. Found file %s - is Bash source file",
                        node->node_id, filename);

            run_file(node, filename, dirname, BASH);
        }

    }

    return 1;
}

void *node_thread(void *arg)
{
    node_t *node = (node_t *)arg;

    node_set_state(node, SEARCHING);

    scandirectory(node, node->data, node->dir);

    trace_info("Node <%d>: completed", node->node_id);

    node_set_state(node, INITED);

    return NULL;
}

void *worker_thread(void *arg)
{
    FILE *file_fd = NULL;
    node_data_t node_data;
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

    worker_set_state(STARTED);

    while (worker_state != STOPPED)
    {
#ifdef USE_THREAD
        node_t *p_node;

        rand_time = rand() % ONE_SEC;
        usleep(rand_time);
#endif

        switch(worker_state)
        {
            case STARTED:
            {
                if (fscanf(file_fd, "%s", input_data) == EOF)
                {
                    trace_info("Detect end of file. Deactivate threads");
#ifdef USE_THREAD
                    worker_set_state(SOFT_STOP);
#else
                    worker_set_state(STOPPED);
#endif
                    break;
                }

                trace_info("command after %.3f sec: %s",
                           (float)rand_time/ONE_SEC, input_data);

#ifdef USE_THREAD
                p_node = node_get_free(&node_data);
                if (!p_node)
                {
                    trace_err("Failed to get free node. '%s' skipped", input_data);
                    continue;
                }

                memcpy (p_node->data, input_data, MAX_INPUT_STR);
                strcpy (p_node->dir, START_DIR);

                node_set_state(p_node, LAUNCHED);

                pthread_create(&p_node->node_th, NULL, node_thread, (void *)p_node);
#else
                node_t node;
                node.node_id = 0;
                memcpy (node.data, input_data, MAX_INPUT_STR);
                strcpy (node.dir, START_DIR);

                scandirectory(&node, node.data, node.dir);
#endif
            }
            break;
            case SOFT_STOP:
#ifdef USE_THREAD
                node_clear_all(&node_data, 0);
                worker_set_state(STOPPED);
#endif
            break;
            case HARD_STOP:
#ifdef USE_THREAD
                node_clear_all(&node_data, 1);
                worker_set_state(STOPPED);
#endif
            break;
            case STOPPED:
            break;
        }
    }

    fclose(file_fd);

    return NULL;
}

void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGQUIT:
            worker_set_state(SOFT_STOP);
        break;

        case SIGINT:
        case SIGTERM:
            worker_set_state(HARD_STOP);
        break;
    }
}

int main()
{
    pthread_t worker_th;
    char data_file[] = "data_file";

    signal(SIGQUIT, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

#ifdef USE_THREAD
    trace_info("Work crew started. Mode: Threads");
#else
    trace_info("Work crew started. Mode: Successively");
#endif

    pthread_create (&worker_th, NULL, worker_thread, (void *)data_file);

    pthread_join (worker_th, NULL);

    trace_info("Work crew finished");

    return 0;
}
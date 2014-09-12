#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define log(fmt, ...) \
            do { fprintf(stdout, "%s:%d:%s(): " fmt"\n", __FILE__, \
                        __LINE__, __func__, ##__VA_ARGS__); } while (0)

#define INPUT_SRC_FILE ""

enum err_code { SUCCESS = 0,
                FAILED,
              };

static char separator_base[] = {};

/* NOTE: all 'int' types must be replace to uint16_t/uint32_t */
typedef struct {
    int of_bkt; /*opened brace*/
    int cf_bkt; /*closed brace*/

    int o_bkt; /*opened bracket*/
    int c_bkt; /*closed bracket*/


} syntax_t;

syntax_t input_syntax = {0};


static int separator_analyze (char *input)
{
    if (!input)
        return FAILED;



    if (input)


    return SUCCESS;
}

int main(void)
{
    FILE *input_file = NULL;

    input_file = fopen (INPUT_SRC_FILE, "r");
    if (input_file == NULL)
    {
        log("failed to open '%s': %s", INPUT_SRC_FILE, strerror(errno));
        return FAILED;
    }

    while ()
    {

    }


    fclose (input_file);

    return SUCCESS;
}

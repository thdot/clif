
#include "clif.h"

#include <stdio.h>


static int clif_cmp_cmd_descriptor(struct clif_cmd_descriptor * d1, struct clif_cmd_descriptor * d2)
{
    return strcmp(d1->command, d2->command);
}

struct clif_cmdtree clif_cmdtree_root = RB_INITIALIZER(&clif_cmdtree_root);

RB_GENERATE(clif_cmdtree, clif_cmd_descriptor, descriptor, clif_cmp_cmd_descriptor)


void clif_add_cmd_descriptor(const char* command, void* functionToCall,
        void (* caller)(struct clif_cmd_descriptor *), short numberOfParameters,
        const char* doc, const char* cmdgroup
        _CLIF_ADD_CMD_PARAM_DECL(), void* end )
{
    struct clif_cmd_descriptor* new_descriptor = malloc(sizeof(struct clif_cmd_descriptor));
    memset(new_descriptor, 0x00, sizeof(struct clif_cmd_descriptor));
    new_descriptor->command = strdup(command);
    new_descriptor->functionToCall = functionToCall;
    new_descriptor->caller = caller;
    new_descriptor->numberOfParameters = numberOfParameters;
    new_descriptor->helptext = strdup(doc);
    RB_INSERT(clif_cmdtree, &clif_cmdtree_root, new_descriptor);
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_ADD_PARAM_DESCR, new_descriptor);
}


int clif_parse(char * line)
{
    int i = 0, result = 0;
    struct clif_cmd_descriptor find, *descr;
    char * token;
    char * parseline = strdup(line);

    token = strtok(parseline, " ");
    if (token == NULL)
        return 0;

    find.command = token;
    descr = RB_FIND(clif_cmdtree, &clif_cmdtree_root, &find);

    if (! descr) {
        printf("command not found: %s\n", token);
        return 1;
    }

    token = strtok(NULL, " ");
    while (token != NULL) {
        if (i + 1 > descr->numberOfParameters) {
            printf("too many number of arguments\n");
            result = 1;
            goto out;
        }
        descr->params[i].value = malloc(descr->params[i].size);
        if ((*descr->params[i].parser)(token, descr->params[i].value) != 0) {
            printf("invalid argument: %s\n", token);
            result = 2;
            goto out;
        }
        token = strtok(NULL, " ");
        i++;
    }
    if (i != descr->numberOfParameters) {
        printf("invalid number of arguments\n");
        result = 1;
        goto out;
    }

    (*descr->caller)(descr);

out:
    for (i = 0; i < descr->numberOfParameters; i++) {
        free(descr->params[i].value);
        descr->params[i].value = NULL;
    }
    free(parseline);

    return result;
}

int clif_parse_int(char * token, int * result)
{
    return sscanf(token, "%d", result) != 1;
}

int clif_parse_ptr(char * token, void * result)
{
    return 1;
}

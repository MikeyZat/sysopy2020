// @author: Mikolaj Zatorski, 2020

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

blocks_array *exec_create_table(char *size_as_string)
{
    int size = (int)strtol(size_as_string, NULL, 10);
    blocks_array *new_blocks_tab = create_files_tab(size);
    return new_blocks_tab;
}

void exec_compare_pairs(char *files_as_chars)
{
    int str_size = strlen(files_as_chars);
    char files_str[str_size]; // needed to avoid segmentation fault
    strcpy(files_str, files_as_chars);
    calculate_diff(create_files_sequence(files_str));
}

int exec_create_blocks(char *file_name, blocks_array *blocks_tab)
{
    return create_operations_blocks(file_name, blocks_tab);
}

void exec_remove_block(char *index_as_str, blocks_array *blocks_tab)
{
    int index = atoi(index_as_str);
    delete_file_block(index, blocks_tab);
}

void exec_remove_operation(
    char *block_index_as_str, char *operation_index_as_str, blocks_array *blocks_tab)
{
    int block_index = atoi(block_index_as_str);
    int op_index = atoi(operation_index_as_str);
    block *curr_block = blocks_tab->blocks[block_index];
    delete_operation_block(op_index, curr_block);
}

int main(int args_num, char *args[])
{
    blocks_array *main_tab;
    int i = 1;
    while (i < args_num)
    {
        char *command = args[i];

        if (strcmp(command, "create_table") == 0)
        {
            main_tab = exec_create_table(args[i + 1]);
            i += 2;
        }
        else if (strcmp(command, "compare_pairs") == 0)
        {
            exec_compare_pairs(args[i + 1]);
            i += 2;
        }
        else if (strcmp(command, "create_blocks") == 0)
        {
            exec_create_blocks(args[i + 1], main_tab);
            i += 2;
        }
        else if (strcmp(command, "remove_block") == 0)
        {
            exec_remove_block(args[i + 1], main_tab);
            i += 2;
        }
        else if (strcmp(command, "remove_operation") == 0)
        {
            exec_remove_operation(args[i + 1], args[i + 2], main_tab);
            i += 3;
        }
        else
        {
            i++;
        }
    }

    return 0;
}

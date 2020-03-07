// @author: Mikolaj Zatorski, 2020

#include <string.h>
#include "library.h"
#include <stdlib.h>
#include <stdio.h>

const int MAX_ROW_LENGTH = 10000;
const int MAX_FILE_LENGTH = 1000000;

blocks_array *create_files_tab(int size)
{
    blocks_array *files_tab = (blocks_array *)calloc(1, sizeof(blocks_array));
    files_tab->size = files_tab->tab_size = size;
    files_tab->blocks = (block **)calloc(size, sizeof(block *));
    return files_tab;
}

files_sequence *create_files_sequence(char files_string[]) // returns object with file names in format of
{                                                          // array [n][2]
    char files_string_copy[strlen(files_string)];
    strcpy(files_string_copy, files_string); // copy string so we will use that later

    char *file_name = strtok(files_string, " :");
    int size = 0;
    while (file_name != NULL) // calculate number of files to init res array
    {
        file_name = strtok(NULL, " :");
        size++;
    }

    size /= 2; // res array is size [n][2]

    // init result structure
    files_sequence *new_files_seq = calloc(1, sizeof(files_sequence));
    new_files_seq->size = size;
    new_files_seq->files = calloc(size, sizeof(char **));

    // start putting filenames into result array
    file_name = strtok(files_string_copy, " :");
    int i = 0;
    while (file_name != NULL)
    { // init space for row
        new_files_seq->files[i] = calloc(2, sizeof(char *));
        // enter first name
        new_files_seq->files[i][0] = calloc(1, strlen(file_name));
        strcpy(new_files_seq->files[i][0], file_name);
        file_name = strtok(NULL, " :");
        // enter second name
        new_files_seq->files[i][1] = calloc(1, strlen(file_name));
        strcpy(new_files_seq->files[i][1], file_name);
        file_name = strtok(NULL, " :");
        i++;
    }
    return new_files_seq;
}

void calculate_diff(files_sequence *files)
{
    char ***files_tab = files->files;
    int size = files->size;

    for (int i = 0; i < size; i++)
    {
        char **curr_files = files_tab[i];
        // prepare create tmp file command
        char *tmp_file_name = calloc(1, strlen(curr_files[0]) + strlen(curr_files[1]) + 2);
        strcpy(tmp_file_name, curr_files[0]);
        strcat(tmp_file_name, ":");
        strcat(tmp_file_name, curr_files[1]); // file1.txt:file2.txt -- name of the tmp file
        char *rm_command = calloc(1, strlen(tmp_file_name) + 3);
        char *touch_command = calloc(1, strlen(tmp_file_name) + 6);
        strcpy(rm_command, "rm ");       // rm file1.txt:file2.txt -- remove old version of tmp file
        strcpy(touch_command, "touch "); // touch file1.txt:file2.txt -- create new tmp file
        strcat(rm_command, tmp_file_name);
        strcat(touch_command, tmp_file_name);
        system(rm_command);
        system(touch_command); // create tmp file
        // prepare command
        char tmp[12 + strlen(curr_files[0]) + strlen(curr_files[1])];
        strcpy(tmp, "diff ");
        strcat(tmp, curr_files[0]);
        strcat(tmp, " ");
        strcat(tmp, curr_files[1]);
        strcat(tmp, " >> ");
        strcat(tmp, tmp_file_name); // diff file1.txt file2.txt >> file1.txt:file2.txt
        // execute command
        system(tmp);
    }
}

int create_operations_blocks(char *file_name, blocks_array *tab_of_blocks)
{

    int index_to_insert = 0;
    while (tab_of_blocks->blocks[index_to_insert] != NULL && index_to_insert < tab_of_blocks->tab_size)
    {
        index_to_insert++;
    }
    // opening the file
    FILE *result_file = fopen(file_name, "r");
    if (result_file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    int i = 0; // this is num of rows in our file
    int operations_size = -1;
    char *line = NULL;                                            // this is our 'reader'
    size_t _ = 0;                                                 // this is not used
    char **tmp = (char **)calloc(MAX_ROW_LENGTH, sizeof(char *)); // file text
    while (getline(&line, &_, result_file) != -1)
    {
        tmp[i] = calloc(MAX_ROW_LENGTH, sizeof(char));
        strcpy(tmp[i], "");
        strcat(tmp[i], line);
        i++;
    }
    // close file and remove unused variable
    fclose(result_file);
    free(line);
    // result structure
    block *result_block = calloc(1, sizeof(block));
    char **operations = calloc(i, sizeof(char *));
    int j = 0; // index in tmp
    while (j < i)
    {
        char *curr_line = tmp[j];
        int x = 0;
        int size_of_curr_line = strlen(curr_line);
        while (x < size_of_curr_line)
        {
            if (curr_line[x] >= '0' && curr_line[x] <= '9')
            {
                // create new operations block
                operations_size++;
                operations[operations_size - 1] = calloc(MAX_ROW_LENGTH, sizeof(char));
                strcpy(operations[operations_size - 1], "");
                break;
            }
            x++;
        }
        if (operations_size > 0)
        {
            strcat(operations[operations_size - 1], curr_line); // append to latest operation block
        }
        j++; // read next line from file
    }
    result_block->size = result_block->tab_size = operations_size;

    tab_of_blocks->blocks[index_to_insert] = result_block;
    tab_of_blocks->size++;
    if (index_to_insert == tab_of_blocks->tab_size)
    {
        tab_of_blocks->tab_size++;
    }

    return index_to_insert;
}

int get_operation_number(blocks_array *blocks_tab, int file_block_index)
{
    if (blocks_tab->size <= file_block_index || blocks_tab->blocks[file_block_index] == NULL)
    {
        printf("Can not get operations block number because the files block\
        with index %d doesn't exist",
               file_block_index);
        return 0;
    }
    return blocks_tab->blocks[file_block_index]->size;
}

void delete_file_block(int block_index, blocks_array *blocks_tab)
{
    if (block_index >= blocks_tab->size)
    {
        printf("Can not delete file block at index %d because\
        it doesn't exist",
               block_index);
    }
    else
    {
        block *ops_to_remove = blocks_tab->blocks[block_index];
        for (int i = 0; i < ops_to_remove->tab_size; i++)
        {
            delete_operation_block(i, ops_to_remove); // remove all operations
        }                                             // inside the file block
        free(blocks_tab->blocks[block_index]);        // remove the operations array
        blocks_tab->size--;
        blocks_tab->blocks[block_index] = NULL; // remove the pointer
    }
}

void delete_operation_block(int op_index, block *operations_tab)
{
    if (operations_tab->size <= op_index || operations_tab->operations[op_index] == NULL)
    {
        printf("Can not delete the operation block with index %d \
         because it doesn't exist.",
               op_index);
    }
    else
    {
        free(operations_tab->operations[op_index]); // remove the string
        operations_tab->size--;
        operations_tab->operations[op_index] = NULL; // remove the pointer
    }
}

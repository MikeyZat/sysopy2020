// @author: Mikolaj Zatorski, 2020

#ifndef LIB_H
#define LIB_H

typedef struct block
{
    int size;
    int tab_size;
    char **operations; // array of strings
} block;

typedef struct blocks_array
{
    int size;
    int tab_size;
    block **blocks;
} blocks_array;

typedef struct files_sequence
{
    int size;
    char ***files; // 2 dimensions array of strings
} files_sequence;

blocks_array *create_files_tab(int size);

files_sequence *create_files_sequence(char *files_string);

void calculate_diff(files_sequence *files);

int create_operations_blocks(char *file_name, blocks_array *tab_of_blocks);

int get_operation_number(blocks_array *tab, int operation_index);

void delete_file_block(int index, blocks_array *tab);

void delete_operation_block(int op_index, block *bl);

#endif
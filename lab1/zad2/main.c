// @author: Mikolaj Zatorski, 2020

#include "library.h"
#include <stdio.h>

int main(int argc, char *args[])
{
    blocks_array *main_tab = create_files_tab(10);
    char files_str[] = "a.txt:b.txt b.txt:c.txt";
    files_sequence *files = create_files_sequence(files_str);
    for (int i = 0; i < files->size; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            printf("%s\n", files->files[i][j]);
        }
    }

    calculate_diff(files);
    int index = create_operations_blocks("a.txt:b.txt", main_tab);
    printf("%d", index);
    return 0;
}

#include <stdio.h>
#include <string.h>

#include "main_lib.h"

int main(int argc, char **argv)
{
#define BUF100 100
    char buf[BUF100];
    FILE *fin = NULL, *fout = NULL;
    int i, len;
    if (argc != 3)
    {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return -1;
    }

    fin  = fopen(argv[1], "r");
    fout = fopen(argv[2], "w");
    if (!fin || !fout) goto RETURN;

    printf("Opened %s for read, %s for write\n", argv[1], argv[2]);
    for (i = 0; fgets(buf, BUF100, fin); ++i)
    {
        len = strlen(buf);
        stem_word(buf, &len);
        buf[len++] = '\n';
        buf[len] = 0;
        fprintf(fout, buf);
    }
    printf("Completed %d\n", i);
RETURN:
    if (fin)  fclose(fin);
    if (fout) fclose(fout);
    return 0;
}

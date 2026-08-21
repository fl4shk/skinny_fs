#include "skinny_fs.h"
#include <stdio.h>

#define MY_TEST_BUF_LEN 1024ul

int main(int argc, char** argv) {
    void* f = skinny_fs_fopen("aa", "w");

    #define TO_WRITE_LEN 3

    const char* to_write[TO_WRITE_LEN] = {
        "This is a test!",
        "Time for some Quirky Questions!",
        "Oh where... is my hairbrush!",
    };

    const size_t MY_TEMP_LEN[TO_WRITE_LEN] = {
        strlen(to_write[0]),
        strlen(to_write[1]),
        strlen(to_write[2]),
    };

    skinny_fs_fwrite(f, to_write[0], sizeof(char) * MY_TEMP_LEN[0]);
    skinny_fs_fclose(f);

    char buf[TO_WRITE_LEN][MY_TEST_BUF_LEN];
    memset(buf[0], 0, sizeof(buf[0]));
    f = skinny_fs_fopen("aa", "r");

    skinny_fs_fread(f, buf[0], sizeof(char) * MY_TEMP_LEN[0]);
    buf[0][MY_TEMP_LEN[0]] = '\0';

    skinny_fs_fclose(f);
    fprintf(
        stderr,
        "contents of \"aa\": %s\n",
        buf[0]
    );

    memset(buf[1], 0, sizeof(buf[1]));

    f = skinny_fs_fopen("b", "w");
    skinny_fs_fwrite(f, to_write[1], sizeof(char) * MY_TEMP_LEN[1]);
    skinny_fs_fclose(f);
    
    f = skinny_fs_fopen("b", "r");
    skinny_fs_fread(f, buf[1], sizeof(char) * MY_TEMP_LEN[1]);
    buf[1][MY_TEMP_LEN[1]] = '\0';
    skinny_fs_fclose(f);

    fprintf(
        stderr,
        "contents of \"b\": %s\n",
        buf[1]
    );

    //snprintf(buf[2], MY_TEST_BUF_LEN, to_write[2]);
    const char* temp_file_name = "./my_file.txt";
    strncpy(buf[2], to_write[2], MY_TEST_BUF_LEN);

    f = skinny_fs_file_init(temp_file_name, buf[2], MY_TEMP_LEN[2]);
    skinny_fs_fclose(f);

    f = skinny_fs_fopen(temp_file_name, "r");
    skinny_fs_fread(f, buf[0], sizeof(char) * MY_TEST_BUF_LEN);
    buf[0][MY_TEST_BUF_LEN - 1] = '\0';
    fprintf(
        stderr,
        "contents of \"%s\": %s\n",
        temp_file_name,
        buf[0]
    );
    skinny_fs_fclose(f);


    return 0;
}

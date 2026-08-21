#ifndef SKINNY_FS_H
#define SKINNY_FS_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum skinny_fs_seek_t
{
    SKINNY_FS_SEEK_CUR = 1,
    SKINNY_FS_SEEK_END = 2,
    SKINNY_FS_SEEK_SET = 0,
} skinny_fs_seek_t;

#define SKINNY_FS_INITIAL_WRITE_FILE_SIZE 1u//1024u // change this if you want

typedef struct skinny_fs_handle_t skinny_fs_handle_t;
typedef struct skinny_fs_file_t skinny_fs_file_t;

struct skinny_fs_file_t {
    bool was_made_by_file_init: 1;

    // a few notes:
    // * we don't support directories
    // * we assume somewhat short-ish paths
    // * only absolute paths (i.e. no relative paths)
    // * thus `filename` specifies the whole filename!
    //const char* filename;
    uint8_t* buf;
    size_t size;
};
struct skinny_fs_handle_t {
    skinny_fs_file_t* f;
    size_t pos;
};

typedef struct skinny_fs_htab_elem_t skinny_fs_htab_elem_t;
typedef struct skinny_fs_htab_vec_t skinny_fs_htab_vec_t;
typedef struct skinny_fs_htab_t skinny_fs_htab_t;

struct skinny_fs_htab_elem_t {
    const char* key;
    void* v;

    //// make this a doubly-linked list
    //skinny_fs_htab_elem_t* prev;
    //skinny_fs_htab_elem_t* next; 
};
struct skinny_fs_htab_vec_t {
    skinny_fs_htab_elem_t* buf;
    size_t buf_size;
};
struct skinny_fs_htab_t {
    skinny_fs_htab_vec_t* vec;
    size_t vec_size_log2;
    size_t most_inner_size;
    //size_t total_size;
};

#ifdef __cplusplus
extern "C" {
#endif      // __cplusplus

//extern skinny_fs_file_t skinny_fs_head;
extern skinny_fs_htab_t* skinny_fs_htab;

//#ifndef SKINNY_FS_BYTE_COUNT_INT
//typedef size_t skinny_fs_byte_count_t;
//#else       // if defined(SKINNY_FS_BYTE_COUNT_INT)
//typedef int skinny_fs_byte_count_t;
//#endif      // SKINNY_FS_BYTE_COUNT_INT

#ifdef SKINNY_FS_SINT_TYPE
typedef SKINNY_FS_SINT_TYPE skinny_fs_sint_t;
#else
typedef long long int skinny_fs_sint_t;
#endif

#ifdef SKINNY_FS_BYTE_COUNT_TYPE
typedef SKINNY_FS_BYTE_COUNT_TYPE skinny_fs_byte_count_t;
#else
typedef size_t skinny_fs_byte_count_t;
#endif

void* skinny_fs_file_init(
    const char* filename, uint8_t* buf, size_t size
);
void* skinny_fs_fopen(
    const char* filename, const char* mode
);
void skinny_fs_fclose(void* handle);
skinny_fs_sint_t skinny_fs_fread(
    void* handle, void* buf, skinny_fs_byte_count_t byte_count
);
skinny_fs_sint_t skinny_fs_fwrite(
    void* handle, const void* buf, skinny_fs_byte_count_t byte_count
);
skinny_fs_sint_t skinny_fs_fseek(
    void* handle, skinny_fs_sint_t offset, skinny_fs_seek_t origin
);
skinny_fs_sint_t skinny_fs_ftell(void* handle);
skinny_fs_sint_t skinny_fs_feof(void* handle);
skinny_fs_sint_t skinny_fs_rename(
    const char* old_filename, const char* new_filename
);

#ifdef __cplusplus
} // extern "C"
#endif      // __cplusplus

#endif      // SKINNY_FS_H

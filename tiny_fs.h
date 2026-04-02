#ifndef TINY_FS_H
#define TINY_FS_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum tiny_fs_seek_t
{
    TINY_FS_SEEK_CUR = 1,
    TINY_FS_SEEK_END = 2,
    TINY_FS_SEEK_SET = 0,
} tiny_fs_seek_t;

#define TINY_FS_INITIAL_WRITE_FILE_SIZE 1u//1024u // change this if you want

typedef struct tiny_fs_handle_t tiny_fs_handle_t;
typedef struct tiny_fs_file_t tiny_fs_file_t;

struct tiny_fs_file_t {
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
struct tiny_fs_handle_t {
    tiny_fs_file_t* f;
    size_t pos;
};

typedef struct tiny_fs_htab_elem_t tiny_fs_htab_elem_t;
typedef struct tiny_fs_htab_vec_t tiny_fs_htab_vec_t;
typedef struct tiny_fs_htab_t tiny_fs_htab_t;

struct tiny_fs_htab_elem_t {
    const char* key;
    void* v;

    //// make this a doubly-linked list
    //tiny_fs_htab_elem_t* prev;
    //tiny_fs_htab_elem_t* next; 
};
struct tiny_fs_htab_vec_t {
    tiny_fs_htab_elem_t* buf;
    size_t buf_size;
};
struct tiny_fs_htab_t {
    tiny_fs_htab_vec_t* vec;
    size_t vec_size_log2;
    size_t most_inner_size;
    //size_t total_size;
};

#ifdef __cplusplus
extern "C" {
#endif      // __cplusplus

//extern tiny_fs_file_t tiny_fs_head;
extern tiny_fs_htab_t* tiny_fs_htab;

//#ifndef TINY_FS_BYTE_COUNT_INT
//typedef size_t tiny_fs_byte_count_t;
//#else       // if defined(TINY_FS_BYTE_COUNT_INT)
//typedef int tiny_fs_byte_count_t;
//#endif      // TINY_FS_BYTE_COUNT_INT

#ifdef TINY_FS_SINT_TYPE
typedef TINY_FS_SINT_TYPE tiny_fs_sint_t;
#else
typedef long long int tiny_fs_sint_t;
#endif

#ifdef TINY_FS_BYTE_COUNT_TYPE
typedef TINY_FS_BYTE_COUNT_TYPE tiny_fs_byte_count_t;
#else
typedef size_t tiny_fs_byte_count_t;
#endif

void* tiny_fs_file_init(
    const char* filename, uint8_t* buf, size_t size
);
void* tiny_fs_fopen(
    const char* filename, const char* mode
);
void tiny_fs_fclose(void* handle);
tiny_fs_sint_t tiny_fs_fread(
    void* handle, void* buf, tiny_fs_byte_count_t byte_count
);
tiny_fs_sint_t tiny_fs_fwrite(
    void* handle, const void* buf, tiny_fs_byte_count_t byte_count
);
tiny_fs_sint_t tiny_fs_fseek(
    void* handle, tiny_fs_sint_t offset, tiny_fs_seek_t origin
);
tiny_fs_sint_t tiny_fs_ftell(void* handle);
tiny_fs_sint_t tiny_fs_feof(void* handle);

#ifdef __cplusplus
} // extern "C"
#endif      // __cplusplus

#endif      // TINY_FS_H

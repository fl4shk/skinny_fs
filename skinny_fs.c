#include "skinny_fs.h"
#include <stdio.h>

static uint32_t _skinny_fs_str_hash_u32(
    const char* key, uint32_t mod_lshift
) {
    uint32_t ret = 0;
    const uint32_t TEMP_KEY_LEN = strlen(key);
    const uint32_t MASK = (((uint32_t)1ul) << mod_lshift) - 1;
    for (uint32_t i=0; i<TEMP_KEY_LEN; ++i) {
        ret ^= (uint32_t)key[i];
        ret = (ret << (uint32_t)5u) | (ret >> (uint32_t)27u);
        ret &= MASK;
    }
    return ret;
}
static uint64_t _skinny_fs_str_hash_u64(
    const char* key, uint64_t mod_lshift
) {
    uint64_t ret = 0;
    const uint64_t TEMP_KEY_LEN = strlen(key);
    const uint64_t MASK = (((uint64_t)1ul) << mod_lshift) - 1;
    for (uint64_t i=0; i<TEMP_KEY_LEN; ++i) {
        ret ^= (uint64_t)key[i];
        ret = (ret << (uint64_t)5ull) | (ret >> (uint64_t)59ull);
        ret &= MASK;
    }
    return ret;
}
static size_t _skinny_fs_str_hash (
    const char* key, size_t mod_lshift
) {
    if (sizeof(mod_lshift) >= sizeof(uint64_t)) {
        return (size_t)_skinny_fs_str_hash_u64(key, mod_lshift);
    } else {
        return (size_t)_skinny_fs_str_hash_u32(key, (uint32_t)mod_lshift);
    }
}

#define SKINNY_FS_HTAB_INITIAL_SIZE_LOG2 ((size_t)2u/*8u*/)
//#define SKINNY_FS_HTAB_INITIAL_SIZE
//    (((size_t)1u) << (SKINNY_FS_HTAB_INITIAL_SIZE_LOG2))
skinny_fs_htab_t* skinny_fs_htab = NULL;


static skinny_fs_htab_vec_t* _skinny_fs_htab_vec_search_shared(
    skinny_fs_htab_t* some_htab, const char* key
) {
    if (some_htab == NULL) {
        return NULL;
    }
    const size_t hash = _skinny_fs_str_hash(
        key,
        some_htab->vec_size_log2
    );
    skinny_fs_htab_vec_t* ret = some_htab->vec + hash;
    return ret;
}
static skinny_fs_htab_elem_t* _skinny_fs_htab_search_for_elem(
    skinny_fs_htab_t* some_htab, const char* key
) {
    if (some_htab == NULL) {
        return NULL;
    }
    skinny_fs_htab_vec_t* vec = _skinny_fs_htab_vec_search_shared(
        some_htab,
        key
    );
    for (size_t i=0; i<vec->buf_size; ++i) {
        skinny_fs_htab_elem_t* item = vec->buf + i;
        if (
            item->key != NULL
            && strcmp(item->key, key) == 0
        ) {
            //return item->v;
            return item;
        }
    }
    return NULL;
}

static inline skinny_fs_file_t* _skinny_fs_htab_search_shared(
    skinny_fs_htab_t* some_htab, const char* key
) {
    skinny_fs_htab_elem_t* my_elem = _skinny_fs_htab_search_for_elem(
        some_htab, key
    );
    if (my_elem != NULL) {
        return my_elem->v;
    }
    return NULL;
}
static void _skinny_fs_htab_insert_shared(
    skinny_fs_htab_t* some_htab,
    const char* key,
    skinny_fs_file_t* to_insert
) {
    //const size_t hash = _skinny_fs_str_hash
    skinny_fs_htab_vec_t* vec = _skinny_fs_htab_vec_search_shared(
        some_htab,
        key//,
        //to_insert->filename
    );
    const size_t old_last_idx = vec->buf_size;
    ++vec->buf_size;
    if (vec->buf == NULL) {
        vec->buf = calloc(
            vec->buf_size,
            sizeof(skinny_fs_htab_elem_t)
        );
    } else {
        vec->buf = realloc(
            vec->buf,
            sizeof(skinny_fs_htab_elem_t) * vec->buf_size
        );
    }
    skinny_fs_htab_elem_t* temp = vec->buf + old_last_idx;
    temp->key = key;
    temp->v = to_insert;
    if (some_htab->most_inner_size < vec->buf_size) {
        some_htab->most_inner_size = vec->buf_size;
    }
}

static void _skinny_fs_htab_maybe_rehash(bool keep_current_size) {
    if (skinny_fs_htab == NULL) {
        skinny_fs_htab = (skinny_fs_htab_t*)calloc(
            1ul,
            sizeof(skinny_fs_htab_t)
        );
        skinny_fs_htab->vec_size_log2 = SKINNY_FS_HTAB_INITIAL_SIZE_LOG2;
        skinny_fs_htab->most_inner_size = (size_t)0u;

        skinny_fs_htab->vec = (
            (skinny_fs_htab_vec_t*)calloc(
                ((size_t)1ul) << skinny_fs_htab->vec_size_log2,
                sizeof(skinny_fs_htab_vec_t)
            )
        );
        return;
    }

    const size_t prev_buf_size_log2 = skinny_fs_htab->vec_size_log2;
    const size_t prev_buf_size = ((size_t)1u) << prev_buf_size_log2;

    if (
        skinny_fs_htab->most_inner_size > (prev_buf_size >> 1)
        || keep_current_size
    ) {
        // at this point we decide to rehash...
        // maybe having (prev_buf_size / 2)
        // is enough of a size to rehash the hash table?
        // I don't know how well this will work in practice.
        // It's admittedly an estimated guess as to something
        // that might work somewhat well.

        const size_t next_buf_size_log2 = (
            keep_current_size
            ? (prev_buf_size_log2 + 1)
            : prev_buf_size_log2
        );
        const size_t next_buf_size = ((size_t)1u) << next_buf_size_log2;

        skinny_fs_htab_t* temp_htab = (skinny_fs_htab_t*)malloc(
            // No need to zero-initialize the bytes this time.
            // (i.e. no need for `calloc()` here.)
            sizeof(skinny_fs_htab_t)
        );
        temp_htab->vec = (skinny_fs_htab_vec_t*)calloc(
            next_buf_size,
            sizeof(skinny_fs_htab_vec_t)
        );
        temp_htab->vec_size_log2 = next_buf_size_log2;
        //temp_htab->most_inner_size = skinny_fs_htab->most_inner_size;
        temp_htab->most_inner_size = 0u;

        for (size_t j=0; j<prev_buf_size; ++j) {
            skinny_fs_htab_vec_t* temp_prev_vec = skinny_fs_htab->vec + j;
            if (temp_prev_vec->buf_size > 0) {
                for (size_t i=0; i<temp_prev_vec->buf_size; ++i) {
                    skinny_fs_htab_elem_t* item = temp_prev_vec->buf + i;
                    const char* key = item->key;
                    skinny_fs_file_t* to_insert = item->v;
                    _skinny_fs_htab_insert_shared(temp_htab, key, to_insert);
                }
                free(temp_prev_vec); 
            }
        }

        free(skinny_fs_htab);
        skinny_fs_htab = temp_htab;
    }
}
static inline void _skinny_fs_htab_insert(
    const char* key,
    skinny_fs_file_t* to_insert
) {
    _skinny_fs_htab_maybe_rehash(false);
    _skinny_fs_htab_insert_shared(skinny_fs_htab, key, to_insert);
}

static inline skinny_fs_file_t* _skinny_fs_htab_search(const char* key) {
    return _skinny_fs_htab_search_shared(skinny_fs_htab, key);
}


static inline skinny_fs_file_t* _skinny_fs_file_search(
    const char* filename
) {
    return (skinny_fs_file_t*)_skinny_fs_htab_search(filename);
}

static skinny_fs_file_t* _skinny_fs_file_maybe_copy_buf(
    skinny_fs_file_t* restrict some_file
) {
    skinny_fs_file_t* ret = some_file;

    if (ret->was_made_by_file_init) {
        uint8_t* temp_buf = (uint8_t*)malloc(sizeof(uint8_t) * ret->size);
        memcpy(temp_buf, ret->buf, ret->size);
        ret->buf = temp_buf;
        ret->was_made_by_file_init = false;
    }

    return ret;
}
static inline void* _skinny_fs_file_create_finish(
    const char* filename, skinny_fs_file_t* f
) {
    //f->is_write = is_write;
    skinny_fs_handle_t* ret = (
        (skinny_fs_handle_t*)malloc(sizeof(skinny_fs_handle_t))
    );
    ret->f = f;
    //ret->is_write = is_write;
    ret->pos = 0u;
    //_skinny_fs_file_insert_before(&skinny_fs_head, ret->f);
    _skinny_fs_htab_insert(filename, ret->f);

    return ret;
}

void* skinny_fs_file_init(const char* filename, uint8_t* buf, size_t size) {
    // NOTE: this is similar conceptually to `fmemopen()`
    skinny_fs_file_t* f = _skinny_fs_file_search(filename);
    if (f != NULL) {
        return NULL;
    }
    f = (skinny_fs_file_t*)malloc(sizeof(skinny_fs_file_t));

    f->was_made_by_file_init = true;

    //f->filename = filename;
    f->buf = buf;
    f->size = size;
    //f->prev = NULL;
    //f->next = NULL;
    //_skinny_fs_file_insert_before(&skinny_fs_head, f);

    //skinny_fs_handle_t* ret = (
    //    (skinny_fs_handle_t*)malloc(sizeof(skinny_fs_handle_t))
    //);
    //ret->f = f;
    //ret->pos = 0u;
    //_skinny_fs_htab_insert(filename, ret->f);

    //return ret;
    return _skinny_fs_file_create_finish(filename, f);
}

void* skinny_fs_fopen(const char* filename, const char* mode) {
    bool is_write;

    // checking `mode[0] == ...` like this is a hack.
    if (mode[0] == 'r') {
        is_write = false;
    } else if (mode[0] == 'w') {
        is_write = true;
    } else {
        // eek!
        return NULL;
    }

    skinny_fs_file_t* f = _skinny_fs_file_search(filename);
    if (f == NULL) {
        if (!is_write) {
            return NULL;
        }
        f = (skinny_fs_file_t*)malloc(sizeof(skinny_fs_file_t));
        //f->filename = filename;
        f->buf = (uint8_t*)malloc(
            sizeof(uint8_t) * SKINNY_FS_INITIAL_WRITE_FILE_SIZE
        );
        f->size = SKINNY_FS_INITIAL_WRITE_FILE_SIZE;
    } else {
        if (is_write) {
            if (!f->was_made_by_file_init) {
                f->buf = (uint8_t*)realloc(
                    f->buf,
                    sizeof(uint8_t) * SKINNY_FS_INITIAL_WRITE_FILE_SIZE
                );
                f->size = SKINNY_FS_INITIAL_WRITE_FILE_SIZE;
            } else {
                _skinny_fs_file_maybe_copy_buf(f);
            }
        }
    }

    return _skinny_fs_file_create_finish(filename, f);
}

void skinny_fs_fclose(void* handle) {
    skinny_fs_handle_t* self = (skinny_fs_handle_t*)handle;
    free(self);
}
skinny_fs_sint_t skinny_fs_fread(
    void* handle, void* buf, skinny_fs_byte_count_t byte_count
) {
    skinny_fs_handle_t* self = (skinny_fs_handle_t*)handle;
    if (byte_count <= 0) {
        return 0;
    } else if (self->pos + byte_count <= self->f->size) {
        memcpy(buf, self->f->buf + self->pos, byte_count);
        self->pos += byte_count;
        return byte_count;
    } else {
        const size_t temp_size = self->f->size - byte_count;
        memcpy(buf, self->f->buf + self->pos, temp_size);
        self->pos = self->f->size;
        return temp_size;
    }
}
skinny_fs_sint_t skinny_fs_fwrite(
    void* handle, const void* buf, skinny_fs_byte_count_t byte_count
) {
    skinny_fs_handle_t* self = (skinny_fs_handle_t*)handle;
    if (byte_count <= 0) {
        return 0;
    } else if (self->pos + byte_count > self->f->size) {
        _skinny_fs_file_maybe_copy_buf(self->f);

        self->f->buf = (uint8_t*)realloc(
            self->f->buf,
            sizeof(uint8_t) * (self->pos + byte_count)
        );
        self->f->size = self->pos + byte_count;
    }
    memcpy(self->f->buf + self->pos, buf, byte_count);
    self->pos += byte_count;
    return byte_count;
}
skinny_fs_sint_t skinny_fs_fseek(
    void* handle, skinny_fs_sint_t offset, skinny_fs_seek_t origin
) {
    skinny_fs_handle_t* self = (skinny_fs_handle_t*)handle;
    switch (origin) {
    case SKINNY_FS_SEEK_SET:
    {
        // from start of file
        self->pos = offset;
    }
        break;
    case SKINNY_FS_SEEK_END:
    {
        // from end of file
        self->pos = self->f->size + offset;
    }
        break;
    case SKINNY_FS_SEEK_CUR:
    {
        self->pos += offset;
    }
        break;
    }
    return 0;
}
skinny_fs_sint_t skinny_fs_ftell(void* handle) {
    skinny_fs_handle_t* self = (skinny_fs_handle_t*)handle;
    return self->pos;
}
skinny_fs_sint_t skinny_fs_feof(void* handle) {
    skinny_fs_handle_t* self = (skinny_fs_handle_t*)handle;
    return self->pos >= self->f->size;
}
skinny_fs_sint_t skinny_fs_rename(
    const char* old_filename, const char* new_filename
) {
    //skinny_fs_file_t* file = _skinny_fs_file_search(old_filename);
    skinny_fs_htab_elem_t* temp = _skinny_fs_htab_search_for_elem(
        skinny_fs_htab, old_filename
    );
    if (temp != NULL) {
        temp->key = new_filename;
        _skinny_fs_htab_maybe_rehash(true);
        return 0;
    }
    return (skinny_fs_sint_t)(-1);
}

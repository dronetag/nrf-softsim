#ifndef _F_CACHE_H_
#define _F_CACHE_H_

#include <onomondo/softsim/list.h>
#include <stdint.h>
#include <stdbool.h>

#define FS_READ_ONLY (1UL << 8)
#define FS_COMMIT_ON_CLOSE (1UL << 7)  // commit changes to NVS on close

struct cache_ctx;

struct cache_entry {
    struct ss_list list;
    uint16_t key;      // NVS key
    uint8_t _flags;    // part of ID is used for flags // TODO
    uint16_t _p;       // local 'file' pointer (ftell, fseek etc)
    uint16_t _l;       // local 'file' length
    uint8_t *buf;      // in case content is cached
    uint16_t _b_size;  // memory allocated for buf
    uint8_t _b_dirty;  // buf is divergent from NVS
    uint8_t _cache_hits;
    char *name;  // path // key for lookup
    void *user;
};

typedef void* (*cache_entry_alloc_cb)(size_t len);
typedef void (*cache_entry_free_cb)(void *);
typedef int (*cache_entry_write_cb)(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len);
typedef int (*cache_entry_read_cb)(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len);
typedef int (*cache_entry_readall_cb)(struct cache_ctx *ctx, struct cache_entry *entry, void **buffer, size_t *len);
typedef int (*cache_entry_remove_cb)(struct cache_ctx *ctx, struct cache_entry *entry);
typedef uint16_t (*cache_entry_length_cb)(struct cache_ctx *ctx, struct cache_entry *entry);

struct cache_strorage_funcs {
    cache_entry_alloc_cb alloc;
    cache_entry_free_cb free;
    cache_entry_write_cb write;
    cache_entry_read_cb read;
    cache_entry_readall_cb readall;
    cache_entry_remove_cb remove;
    cache_entry_length_cb length;
};

#define CACHE_MAX_ENTRIES (0xFFFFFFFF)

struct cache_ctx {
    struct ss_list file_list;
    struct cache_strorage_funcs *storage_f;
    /* Whether the cache should free the entries upon replacement */
    bool free_entries;
    /* Whether the cache should reuse entries . When enabled cache will reject any entries above max_entries*/
    bool static_cache;
    size_t max_entries;
};

/**
 * @brief find a suitable cache entry with a buffer that can be re-used
 *
 */
struct cache_entry *f_cache_find_buffer(struct cache_entry *entry, struct cache_ctx *cache);
struct cache_entry *f_cache_find_by_name(const char *name, struct cache_ctx *cache);
void f_cache_init(struct cache_ctx *cache, bool static_cache, size_t max_entries, struct cache_strorage_funcs *funcs, bool free_entries);
void *f_cache_alloc(struct cache_ctx *cache, size_t len);
void f_cache_free(struct cache_ctx *cache, void *ptr);
bool f_cache_empty(struct cache_ctx *cache);
int f_cache_read_to_cache(struct cache_ctx *cache, struct cache_entry *entry);
int f_cache_fread(struct cache_ctx *cache, struct cache_entry *entry, void *ptr, size_t size, size_t nmemb);
int f_cache_fclose(struct cache_ctx *cache, struct cache_entry *entry);
int f_cache_fseek(struct cache_ctx *cache, struct cache_entry *entry, long offset, int whence);
int f_cache_ftell(struct cache_ctx *cache, struct cache_entry *entry);
int f_cache_putc(struct cache_ctx *cache, struct cache_entry *entry, int c);
int f_cache_remove(struct cache_ctx *cache, struct cache_entry *entry);
size_t f_cache_fwrite(struct cache_ctx *cache, struct cache_entry *entry, const void *prt, size_t size, size_t count);
int f_cache_fopen(struct cache_ctx *cache, struct cache_entry *entry);
void f_cache_close(struct cache_ctx *cache);
int f_cache_create_entry(struct cache_ctx *cache, uint16_t key, char *name, uint16_t flags, struct cache_entry **out_entry);
void f_cache_delete_entry(struct cache_ctx *cache, struct cache_entry *entry);

#endif /* _F_CACHE_H_ */

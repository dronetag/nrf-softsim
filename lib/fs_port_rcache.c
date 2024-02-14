#include <onomondo/softsim/fs_port.h>
#include <onomondo/softsim/utils.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/slist.h>

#include "profile.h"
#include "impl_fs_port.h"
#include "f_cache.h"


// LOG_MODULE_REGISTER(softsim_fs_port, 4);
LOG_MODULE_REGISTER(softsim_fs_port, CONFIG_SOFTSIM_LOG_LEVEL);

#define ALLOC_FILENAME CONFIG_SOFTSIM_FS_PATH_LEN

struct rcache_file {
    bool cached;
    struct cache_entry *entry;
    impl_port_FILE *fp;
};

struct rcache_ctx {
    struct cache_ctx cache;
};

static struct rcache_ctx rcache;

#ifndef SEEK_SET
#define SEEK_SET 0 /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1 /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END 2 /* set file offset to EOF plus offset */
#endif

static int port_fs_cache_entry_write(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len)
{
    ARG_UNUSED(ctx);
    LOG_DBG("FCache write: %s", entry->name);
    impl_port_FILE f = impl_port_fopen(entry->name, "w");
    if(f == NULL) {
        return -EINVAL;
    }
    int rc = impl_port_fwrite(buffer, len, 1, f);
    if(rc != len) {
        return -1;
    }
    impl_port_fclose(f);
    return len;
}

static int port_fs_cache_entry_read(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len)
{
    ARG_UNUSED(ctx);
    LOG_DBG("FCache read: %s", entry->name);
    impl_port_FILE f = impl_port_fopen(entry->name, "r");
    if(f == NULL) {
        return -EINVAL;
    }
    int rc = impl_port_fread(buffer, len, 1, f);
    impl_port_fclose(f);
    return rc;
}

static uint16_t port_fs_cache_entry_length(struct cache_ctx *ctx, struct cache_entry *entry)
{
    ARG_UNUSED(ctx);
    LOG_DBG("FCache length: %s", entry->name);
    /* TODO: Handle errors? */
    /* Open seek tell */
    impl_port_FILE f = impl_port_fopen(entry->name, "r");
    if(f == NULL) {
        return 0;
    }
    int rc = impl_port_fseek(f, 0, SEEK_END);
    if(rc) {
        return 0;
    }
    long size = impl_port_ftell(f);
    impl_port_fclose(f);

    if(size >= 0) {
        return size;
    }
    LOG_ERR("Tell failed: %d", (int)size);
    return 0;
}

static int port_fs_cache_entry_remove(struct cache_ctx *ctx, struct cache_entry *entry)
{
    ARG_UNUSED(ctx);
    LOG_DBG("FCache remove: %s", entry->name);
    return impl_port_remove(entry->name);
}

struct cache_strorage_funcs fs_cache_storage_funcs = {
    .read = port_fs_cache_entry_read,
    .write = port_fs_cache_entry_write,
    .length = port_fs_cache_entry_length,
    .remove = port_fs_cache_entry_remove
};

int init_fs() {
    f_cache_init(&rcache.cache, &fs_cache_storage_funcs);
    return impl_init_fs();
}

/*
 * This will only be called when softsim is deinitialized.
 * I.e. when the modem goes to cfun=0 or cfun=4
 * */
int deinit_fs() {
    f_cache_close(&rcache.cache);
    return impl_deinit_fs();
}

/**
 * @brief Implements a version of standard C fopen.
 * @param path full path.
 * @param mode 
 * @return pointer to a file handle
 */
port_FILE port_fopen(char *path, char *mode) {
    int rc;
    struct cache_entry *entry = f_cache_find_by_name(path, &rcache.cache);
    /* Check whether the file exists already */
    if(entry && entry->user) {
        LOG_ERR("Opening already opened cache file!!!");
        /* TODO: What to do here ? */
        assert(false);
    }
    bool read_only = strcmp(mode, "r") == 0;

    struct rcache_file *cPtr;
    LOG_DBG("Allocating rcache_file: %s", path);
    cPtr = SS_ALLOC(struct rcache_file);
    if(!entry) {
        char *name_alloc = f_cache_alloc(&rcache.cache, strlen(path)+1);
        if(name_alloc) {
            strcpy(name_alloc, path);
            int flags = 0;
            if(read_only) {
                flags |= FS_READ_ONLY;
            } else {
                flags |= FS_COMMIT_ON_CLOSE;
            }
            rc = f_cache_create_entry(&rcache.cache, 0, name_alloc, flags, &entry);
            if(rc) {
                /* If no memory for the cache entry free the name */
                f_cache_free(&rcache.cache, name_alloc);
            }
            /* TODO: Entries are not removed so the names are allocated indefinitly */
        }
    }

    /* Mark whether the allocation was successfull and the file is cached */
    cPtr->cached = entry != NULL;
    cPtr->entry = entry;
    entry->user = cPtr;

    if(cPtr->cached) {
        rc = f_cache_fopen(&rcache.cache, entry);
        if(rc == 0) {
            /* success fopen */
            return cPtr;
        } else if(rc != -ENOMEM) {
            /* Cache is out of space */
            SS_FREE(cPtr);
            return NULL;
        }
        /* Fall into uncached variant */
    }
    cPtr->cached = false;
    cPtr->entry = NULL;

    LOG_DBG("Using uncached variant due to lack of resources");
    /* Need to actualy open the file */
    cPtr->fp = impl_port_fopen(path, mode);
    if(cPtr->fp == NULL) {
        SS_FREE(cPtr);
        return NULL;
    }
    return cPtr;
}

/**
 * @brief Implements a version of standard C fread. 
 *
 * @param ptr destinanion memory
 * @param size size of element
 * @param nmemb number of elements
 * @param fp file pointer
 * @return elements read
 */
size_t port_fread(void *ptr, size_t size, size_t nmemb, port_FILE fp) {
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        return f_cache_fread(&rcache.cache, cPtr->entry, ptr, size, nmemb);
    } else {
        return impl_port_fread(ptr, size, nmemb, cPtr->fp);
    }
}

int port_fclose(port_FILE fp) {
    struct rcache_file *cPtr = fp;
    /* When file is cached do not free or close */
    impl_port_FILE impl_fp = cPtr->fp;

    int rc;
    if(cPtr->cached) {
        /* Remove user from the cache entry */
        cPtr->entry->user = NULL;
        rc = f_cache_fclose(&rcache.cache, cPtr->entry);
    } else {
        rc = impl_port_fclose(impl_fp);
    }
    SS_FREE(cPtr);
    return rc;
}

int port_fseek(port_FILE fp, long offset, int whence) {
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        return f_cache_fseek(&rcache.cache, cPtr->entry, offset, whence);
    } else {
        return impl_port_fseek(cPtr->fp, offset, whence);
    }
}

long port_ftell(port_FILE fp) {
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        return f_cache_ftell(&rcache.cache, cPtr->entry);
    } else {
        return impl_port_ftell(cPtr->fp);
    }
}

int port_fputc(int c, port_FILE fp) {
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        return f_cache_putc(&rcache.cache, cPtr->entry, c);
    } else {
        return impl_port_fputc(c, cPtr->fp);
    }
}

int port_access(const char *path, int amode) {
    return impl_port_access(path, amode);
}

int port_mkdir(const char *path, int _) {
    return impl_port_mkdir(path, _);
}  

int port_remove(const char *path) {
    return impl_port_remove(path);
}

// very unlike to be invoked tbh
int port_rmdir(const char *path) {
    return impl_port_rmdir(path);
}

int port_check_provisioned() {
    return impl_port_check_provisioned();
}

/**
 * @brief Provision the SoftSIM with the given profile
 *
 *
 * @param profile ptr to the profile
 * @param len Len of profile. 332 otherwise invalid.
 */
int port_provision(struct ss_profile *profile) {
    return impl_port_provision(profile);
}

size_t port_fwrite(const void *ptr, size_t size, size_t count, port_FILE fp) {
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        return f_cache_fwrite(&rcache.cache, cPtr->entry, ptr, size, count);
    } else {
        return impl_port_fwrite(ptr, size, count, cPtr->fp);
    }
}

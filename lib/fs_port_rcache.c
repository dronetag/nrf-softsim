#include <onomondo/softsim/fs_port.h>
#include <onomondo/softsim/utils.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <nrf_softsim.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/slist.h>

#include "profile.h"
#include "impl_fs_port.h"
#include "f_cache.h"

#define CACHE_ENTRIES 20
#define PRECACHE_FILES

#include "../littlefs/scripts/ss_static_files.h"
LOG_MODULE_REGISTER(softsim_fs_port, CONFIG_SOFTSIM_LOG_LEVEL);
/* Will write out the contents of profile files for debugging purposes to ensure consistency of these files */
// #define DEBUG_PROFILE_FILES
// #define DEBUG_ALL_FILES
// #define DEBUG_FILE_CONTENT
#define DEBUG_FCACHE_CALLBACKS

#ifdef DEBUG_FCACHE_CALLBACKS
#define FCACHE_LOG_DBG(...) LOG_DBG(__VA_ARGS__)
#else
#define FCACHE_LOG_DBG(...)
#endif


//#define DEBUG_FS_CALLS

#ifdef DEBUG_FS_CALLS
#define FS_LOG_DBG(...) LOG_DBG(__VA_ARGS__)
#else
#define FS_LOG_DBG(...)
#endif

#ifdef DEBUG_PROFILE_FILES
#define IMSI_PATH "/3f00/7ff0/6f07"
#define ICCID_PATH "/3f00/2fe2"
#define A001_PATH "/3f00/a001"
#define A004_PATH "/3f00/a004"

static const char *profile_paths[] = {
    IMSI_PATH,
    ICCID_PATH,
    A001_PATH,
    A004_PATH
};
#endif


#if defined(DEBUG_PROFILE_FILES) || defined(DEBUG_ALL_FILES)
#define DEBUG_FILE_ACCESS
#endif

LOG_MODULE_REGISTER(softsim_fs_port, 4);
// LOG_MODULE_REGISTER(softsim_fs_port, CONFIG_SOFTSIM_LOG_LEVEL);

#define ALLOC_FILENAME CONFIG_SOFTSIM_FS_PATH_LEN

struct rcache_file {
#ifdef DEBUG_FILE_ACCESS
    bool debug;
    const char *path;
#endif
    bool cached;
    struct cache_entry *entry;
    impl_port_FILE *fp;
};

struct rcache_ctx {
    bool initialized;
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
    FCACHE_LOG_DBG("FCache write: %s", entry->name);
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


static int port_fs_cache_entry_readall(struct cache_ctx *ctx, struct cache_entry *entry, void **buffer, size_t *len)
{
    ARG_UNUSED(ctx);
    FCACHE_LOG_DBG("FCache readall: %s", entry->name);
    impl_port_FILE f = impl_port_fopen(entry->name, "r");
    if(f == NULL) {
        return -EINVAL;
    }
    int rc = impl_port_size(f);
    if(rc < 0) {
        return rc;
    }
    *len = rc;
    *buffer = f_cache_alloc(ctx, *len);
    if(*buffer == NULL) {
        return -ENOMEM;
    }

    rc = impl_port_fread(*buffer, *len, 1, f);
    impl_port_fclose(f);
    return rc;
}


static int port_fs_cache_entry_read(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len)
{
    ARG_UNUSED(ctx);
    FCACHE_LOG_DBG("FCache read: %s", entry->name);
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
    FCACHE_LOG_DBG("FCache length: %s", entry->name);
    /* TODO: Handle errors? */
    /* Open seek tell */
    int rc = impl_port_size(entry->name);
    if(rc >= 0) {
        return rc;
    }
    LOG_ERR("FS Size failed: %d", (int)rc);
    return 0;
}

static int port_fs_cache_entry_remove(struct cache_ctx *ctx, struct cache_entry *entry)
{
    ARG_UNUSED(ctx);
    FCACHE_LOG_DBG("FCache remove: %s", entry->name);
    return impl_port_remove(entry->name);
}

struct cache_strorage_funcs fs_cache_storage_funcs = {
    .read = port_fs_cache_entry_read,
    .readall = port_fs_cache_entry_readall,
    .write = port_fs_cache_entry_write,
    .length = port_fs_cache_entry_length,
    .remove = port_fs_cache_entry_remove
};

int init_fs() {
    int rc;
    rc = impl_init_fs();

    if(!rcache.initialized) {
        rcache.initialized = true;
        bool static_cache = false;
#if defined(PRECACHE_FILES) && defined(CONFIG_SOFTSIM_TEMPLATE_GENERATION_CODE)
        static_cache = true;
#endif
        f_cache_init(&rcache.cache, static_cache, CACHE_ENTRIES, &fs_cache_storage_funcs, true);
#if defined(PRECACHE_FILES) && defined(CONFIG_SOFTSIM_TEMPLATE_GENERATION_CODE)
        if(nrf_softsim_check_provisioned()) {
            LOG_INF("Caching files start");
            /* Files already ordered by the cache hit frequency */
            int cached_files = MIN(onomondo_sf_files_len, CACHE_ENTRIES);
            for(int i = 0; i < MIN(onomondo_sf_files_len, CACHE_ENTRIES); i++) {
                LOG_INF("Caching file: %s %d/%d", onomondo_sf_files[i].name, i+1, cached_files);
                /* Why fopen is not const ??? who knows */
                port_FILE f = port_fopen(onomondo_sf_files[i].name, "r");
                if(f == NULL) {
                    LOG_ERR("Failed to precache file: %s", onomondo_sf_files[i].name);
                }
                if(f) {
                    port_fclose(f);
                }
            }
            LOG_INF("Caching end");
        }
#endif
    }
    return rc;
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
    FS_LOG_DBG("Allocating rcache_file: %s", path);
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
#ifdef DEBUG_FILE_ACCESS
#ifdef DEBUG_ALL_FILES
    cPtr->debug = true;
#else
    cPtr->debug = false;
#endif
    cPtr->path = "Unknown";
#ifdef DEBUG_PROFILE_FILES
    for (int i = 0; i < ARRAY_SIZE(profile_paths); i++) {
        if(strcmp(path, profile_paths[i]) == 0) {
            cPtr->debug = true;
            cPtr->path = profile_paths[i];
        }
    }
#endif
#endif
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
            if(entry) {
                f_cache_delete_entry(&rcache.cache, entry);
            }
            SS_FREE(cPtr);
            return NULL;
        }
        /* Fall into uncached variant */
    }

    /* Delete the entry if exists we no longer need it */
    if(entry) {
        f_cache_delete_entry(&rcache.cache, entry);
    }
    cPtr->cached = false;
    cPtr->entry = NULL;

    FS_LOG_DBG("Using uncached variant due to lack of resources");
    LOG_WRN("Using uncached variant due to lack of resources");
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
    int rc;
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        rc = f_cache_fread(&rcache.cache, cPtr->entry, ptr, size, nmemb);
    } else {
        rc = impl_port_fread(ptr, size, nmemb, cPtr->fp);
    }
#ifdef DEBUG_FILE_ACCESS
    if(cPtr->debug) {
#ifdef DEBUG_FILE_CONTENT
        LOG_HEXDUMP_DBG(ptr, size*nmemb, "fread data");
#endif
        LOG_DBG("fread path: %s, rc: %d", cPtr->path, rc);
    }
#endif
    return rc;
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
    int rc;
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        rc = f_cache_fseek(&rcache.cache, cPtr->entry, offset, whence);
    } else {
        rc = impl_port_fseek(cPtr->fp, offset, whence);
    }
#ifdef DEBUG_FILE_ACCESS
    if(cPtr->debug) {
        LOG_DBG("fseek path: %s, off: %ld, whence: %d, rc: %d", cPtr->path, offset, whence, rc);
    }
#endif
    return rc;
}

long port_ftell(port_FILE fp) {
    int rc;
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        rc = f_cache_ftell(&rcache.cache, cPtr->entry);
    } else {
        rc = impl_port_ftell(cPtr->fp);
    }
#ifdef DEBUG_FILE_ACCESS
    if(cPtr->debug) {
        LOG_DBG("ftell path: %s, rc: %d", cPtr->path, rc);
    }
#endif
    return rc;
}

int port_fputc(int c, port_FILE fp) {
    int rc;
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        rc = f_cache_putc(&rcache.cache, cPtr->entry, c);
    } else {
        rc = impl_port_fputc(c, cPtr->fp);
    }
#ifdef DEBUG_FILE_ACCESS
    if(cPtr->debug) {
        LOG_DBG("putc path: %s, c: %d, rc: %d", cPtr->path, c, rc);
    }
#endif
    return rc;
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
    int rc;
    struct rcache_file *cPtr = fp;
    if(cPtr->cached) {
        rc = f_cache_fwrite(&rcache.cache, cPtr->entry, ptr, size, count);
    } else {
        rc = impl_port_fwrite(ptr, size, count, cPtr->fp);
    }
#ifdef DEBUG_FILE_ACCESS
    if(cPtr->debug) {
#ifdef DEBUG_FILE_CONTENT
        LOG_HEXDUMP_DBG(ptr, size*count, "fwrite data");
#endif
        LOG_DBG("fwrite path: %s, size: %d, count: %d, rc: %d", cPtr->path, size, count, rc);
    }
#endif
    return rc;
}

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


LOG_MODULE_REGISTER(softsim_fs_port, 4);
// LOG_MODULE_REGISTER(softsim_fs_port, CONFIG_SOFTSIM_LOG_LEVEL);

#define ALLOC_FILENAME CONFIG_SOFTSIM_FS_PATH_LEN

struct rcache_file {
    sys_snode_t s_node;
    bool cached;
    bool opened;
    char *fileName;
    impl_port_FILE *fp;
};

struct rcache_ctx {
    sys_slist_t cached_files;
    int cached_files_len;
};

static struct rcache_ctx rcache;

int init_fs() {
    rcache.cached_files_len = 0;
    sys_slist_init(&rcache.cached_files);
    return impl_init_fs();
}

/*
 * This will only be called when softsim is deinitialized.
 * I.e. when the modem goes to cfun=0 or cfun=4
 * */
int deinit_fs() {
    sys_snode_t *nPtr;
    /* Find whether the file is already open */
    SYS_SLIST_FOR_EACH_NODE(&rcache.cached_files, nPtr) {
        struct rcache_file *cSPtr = CONTAINER_OF(nPtr, struct rcache_file, s_node);
        sys_slist_find_and_remove(&rcache.cached_files, nPtr);

        /* Close file and free it */
        cSPtr->cached = false;
        port_fclose(cSPtr);
        rcache.cached_files_len--;
    }
    /* Clear the slist just to be sure */
    sys_slist_init(&rcache.cached_files);
    return impl_deinit_fs();
}

#define SEEK_SET	0	/* Seek from beginning of file.  */
void rcache_close_file(struct rcache_file *ptr)
{
    if(ptr->opened) {
        LOG_ERR("Attempting to close cached file that is opened");
        return;
    }
    sys_slist_find_and_remove(&rcache.cached_files, &ptr->s_node);

    /* Close file and free it */
    LOG_DBG("Closing rcache_file: %s", ptr->fileName);
    ptr->cached = false;
    port_fclose(ptr);
    rcache.cached_files_len--;
}

/**
 * @brief Implements a version of standard C fopen.
 * @param path full path.
 * @param mode 
 * @return pointer to a file handle
 */
port_FILE port_fopen(char *path, char *mode) {
    bool cached = strcmp(mode, "r") == 0;

    struct rcache_file *fPtr = NULL;
    sys_snode_t *nPtr;
    /* Find whether the file is already open */
    SYS_SLIST_FOR_EACH_NODE(&rcache.cached_files, nPtr) {
        struct rcache_file *cSPtr = CONTAINER_OF(nPtr, struct rcache_file, s_node);
        if(strcmp(cSPtr->fileName, path) == 0) {
            fPtr = cSPtr;
            break;
        }
    }
    if(!cached && fPtr) {
        LOG_DBG("File: %s closed due to change of mode", path);
        /* Needs to close the file to allow other operation sync */
        rcache_close_file(fPtr);
        fPtr = NULL;
    }

    struct rcache_file *cPtr;
    if(fPtr == NULL) {
        LOG_DBG("Allocating rcache_file: %s", path);
        cPtr = SS_ALLOC(struct rcache_file);
        cPtr->fileName = NULL;
        cPtr->fp = impl_port_fopen(path, mode);
        if(cPtr->fp == NULL) {
            SS_FREE(cPtr);
            return NULL;
        }

        /* Only now activate caching */
        if(cached) {
            LOG_DBG("File rcache_file activated");
            cPtr->fileName = k_malloc(strlen(path)+1);
            strcpy(cPtr->fileName, path);

            if(rcache.cached_files_len >= CONFIG_SOFTSIM_FS_READ_FILE_CACHE_SIZE) {
                LOG_DBG("Searching for file to close");
                struct rcache_file *cSPtr = NULL;
                sys_snode_t *nPtr = NULL;
                SYS_SLIST_FOR_EACH_NODE(&rcache.cached_files, nPtr) {
                    cSPtr = CONTAINER_OF(nPtr, struct rcache_file, s_node);
                    /* Find first file that is not opened */
                    if(!cSPtr->opened) {
                        break;
                    }
                }
                if(cSPtr == NULL) {
                    LOG_ERR("No files in cache should not happen");
                } else {
                    rcache_close_file(cSPtr);
                }
            }
            sys_slist_append(&rcache.cached_files, &cPtr->s_node);
            rcache.cached_files_len++;
        }
    } else {
        cPtr = fPtr;
        /* Rewind the file */
        impl_port_fseek(cPtr->fp, 0, SEEK_SET);


        /* Simple LRU scheme just push to back */
        sys_slist_find_and_remove(&rcache.cached_files, &cPtr->s_node);
        sys_slist_append(&rcache.cached_files, &cPtr->s_node);
    }
    cPtr->opened = true;
    cPtr->cached = cached;
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
    return impl_port_fread(ptr, size, nmemb, cPtr->fp);
}

int port_fclose(port_FILE fp) {
    struct rcache_file *cPtr = fp;
    /* When file is cached do not free or close */
    cPtr->opened = false;
    if(cPtr->cached)
        return 0;
    if(cPtr->fileName) {
        SS_FREE(cPtr->fileName);
        cPtr->fileName = NULL;
    }
    impl_port_FILE impl_fp = cPtr->fp;
    SS_FREE(cPtr);
    return impl_port_fclose(impl_fp);
}

int port_fseek(port_FILE fp, long offset, int whence) {
    struct rcache_file *cPtr = fp;
    return impl_port_fseek(cPtr->fp, offset, whence);
}

long port_ftell(port_FILE fp) {
    struct rcache_file *cPtr = fp;
    return impl_port_ftell(cPtr->fp);
}

int port_fputc(int c, port_FILE fp) {
    struct rcache_file *cPtr = fp;
    return impl_port_fputc(c, cPtr->fp);
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
    return impl_port_fwrite(ptr, size, count, cPtr->fp);
}

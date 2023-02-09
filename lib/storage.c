/*
 * Author: Philipp Maier
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
// #include <linux/limits.h>
#include <onomondo/softsim/file.h>
#include <onomondo/softsim/list.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <onomondo/softsim/utils.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fs_port.h"

/* TODO: Make configurable (config file) */
static char storage_path[] = "/lfs";

// the port be somewhere else... ???? nah.

/* Generate a host filesystem path for a given file path. */
static int gen_abs_host_path(char *def_path, struct ss_list *path, bool def, char *division) {
    char host_fs_path[MAX_FILE_NAME];
    char abs_host_fs_path[MAX_FILE_NAME + 1];
    static char *host_fs_path_ptr;
    struct ss_file *path_cursor;
    struct ss_file *path_last = NULL;
    int rc;

    /* FIXME: These error messages should be moved to the caller functions
     * as requested file content... would als be printed when the file size
     * is requested, which is missleading. */

    if (ss_list_empty(path)) {
        if (def)
            SS_LOGP(SSTORAGE, LERROR,
                    "%s: unable to generate path to load file definition\n", division);
        else
            SS_LOGP(SSTORAGE, LERROR,
                    "%s: unable to generate path to load file content\n", division);
        return -EINVAL;
    }

    memset(host_fs_path, 0, sizeof(host_fs_path));
    host_fs_path_ptr = host_fs_path;

    SS_LIST_FOR_EACH(path, path_cursor, struct ss_file, list) {
        rc = snprintf(host_fs_path_ptr,
                      sizeof(host_fs_path) - (host_fs_path_ptr - host_fs_path),
                      path_cursor->fid > 0xffff ? "/%08x" : "/%04x", path_cursor->fid);
        host_fs_path_ptr += rc;
        path_last = path_cursor;
    }

    if (def) {
        snprintf(abs_host_fs_path, sizeof(abs_host_fs_path), "%s%s.def", storage_path,
                 host_fs_path);
        SS_LOGP(SSTORAGE, LINFO,
                "%s: requested file definition for %04x on host file system : %s\n",
                division, path_last->fid, abs_host_fs_path);
    } else {
        snprintf(abs_host_fs_path, sizeof(abs_host_fs_path), "%s%s", storage_path,
                 host_fs_path);
        SS_LOGP(SSTORAGE, LINFO,
                "%s: requested file content for %04x on host file system: %s\n", division,
                path_last->fid, abs_host_fs_path);
    }

    strncpy(def_path, abs_host_fs_path, MAX_FILE_NAME);
    return 0;
}

/* Read file definition from host file system */
static int read_file_def(char *host_path, struct ss_file *file) {
    nrf_FILE *fd;
    char line_buf[1024];
    char *rc;

    fd = nrf_fopen(host_path, "r");
    if (!fd) {
        SS_LOGP(SSTORAGE, LERROR, "unable to open definition file: %s\n", host_path);
        return -EINVAL;
    }

    rc = nrf_fgets(line_buf, sizeof(line_buf), fd);
    nrf_fclose(fd);
    if (!rc) {
        SS_LOGP(SSTORAGE, LERROR, "unable to read definition file: %s\n", host_path);
        return -EINVAL;
    }

    /* Note: Module fs.c must ensure freeing of the fci data */
    file->fci = ss_buf_from_hexstr(line_buf);

    return 0;
}

/*! Get file definition for file (tip of the path).
 *  \param[out] file user provided memory to file struct to store definition.
 *  \param[in] path path to the file to be read.
 *  \returns 0 on success, -EINVAL on failure */
int ss_storage_get_file_def(struct ss_list *path) {
    /*! Note: This function will allocate memory in file to store the file
     *  definition. The caller must take care of freeing. */

    char host_path[MAX_FILE_NAME + 1];
    struct ss_file *file;
    int rc;

    rc = gen_abs_host_path(host_path, path, true, "get-def");
    if (rc < 0)
        return -EINVAL;

    file = ss_get_file_from_path(path);
    if (!file)
        return -EINVAL;

    return read_file_def(host_path, file);
}

/*! Get content from file (tip of the path).
 *  \param[in] path path to the file to be read.
 *  \param[in] read_offset offset to start reading the file at.
 *  \param[in] read_len length of data to read.
 *  \returns buffer with content data on success, NULL on failure */
struct ss_buf *ss_storage_read_file(struct ss_list *path, size_t read_offset, size_t read_len) {
    /*! Note: This function will allocate memory in fileto store the file
     *  contents. The caller must take care of freeing. */

    /* TODO: check if the path really points to a file. If the path points
     * to a directory print an error and return a size of 0. (Normally this
     * shouldn't happen because the FCP is always checked before calling
     * this function. */

    char host_path[MAX_FILE_NAME + 1];
    int rc;
    nrf_FILE *fd;
    char *line_buf;
    size_t fgets_rc;
    struct ss_buf *result;

    rc = gen_abs_host_path(host_path, path, false, "read");
    if (rc < 0)
        return NULL;

    line_buf = SS_ALLOC_N(read_len * 2 + 1);
    memset(line_buf, 0, read_len * 2 + 1);

    fd = nrf_fopen(host_path, "r");
    if (!fd) {
        SS_LOGP(SSTORAGE, LERROR, "unable to open content file: %s\n", host_path);
        SS_FREE(line_buf);
        return NULL;
    }

    rc = nrf_fseek(fd, read_offset * 2, SEEK_SET);
    if (rc != 0) {
        SS_LOGP(SSTORAGE, LERROR,
                "unable to seek (read_offset=%u) requested data in content file: %s\n",
                read_offset, host_path);
        SS_FREE(line_buf);
        nrf_fclose(fd);
        return NULL;
    }

    fgets_rc = nrf_fread(line_buf, 2, read_len, fd);
    if (fgets_rc != read_len) {
        SS_LOGP(SSTORAGE, LERROR,
                "unable to load content (read_offset=%lu, read_len=%lu) from file: %s\n",
                read_offset, read_len, host_path);
        SS_FREE(line_buf);
        nrf_fclose(fd);
        return NULL;
    }

    nrf_fclose(fd);

    /* Note: Module fs.c must ensure freeing of the content */
    result = ss_buf_from_hexstr(line_buf);
    SS_FREE(line_buf);
    return result;
}

/*! Write data to a file (tip of the path).
 *  \param[in] path path to the file to be written.
 *  \param[in] write_offset offset to start writing the file at.
 *  \param[in] write_len length of data to be written.
 *  \returns 0 on success, -EINVAL on failure. */
int ss_storage_write_file(struct ss_list *path, uint8_t *data, size_t write_offset,
                          size_t write_len) {
    char host_path[MAX_FILE_NAME + 1];
    int rc;
    nrf_FILE *fd;
    size_t i;
    char hex[3];
    size_t fwrite_rc;

    rc = gen_abs_host_path(host_path, path, false, "write");
    if (rc < 0)
        return -EINVAL;

    fd = nrf_fopen(host_path, "r+");
    if (!fd) {
        SS_LOGP(SSTORAGE, LERROR, "unable to open content file: %s\n", host_path);
        return -EINVAL;
    }

    rc = nrf_fseek(fd, write_offset * 2, SEEK_SET);
    if (rc != 0) {
        SS_LOGP(SSTORAGE, LERROR,
                "unable to seek (write_offset=%lu) data to content file: %s\n",
                write_offset, host_path);
        nrf_fclose(fd);
        return -EINVAL;
    }

    for (i = 0; i < write_len; i++) {
        snprintf(hex, sizeof(hex), "%02x", data[i]);
        fwrite_rc = nrf_fwrite(hex, sizeof(hex) - 1, 1, fd);
        if (fwrite_rc != 1) {
            SS_LOGP(SSTORAGE, LERROR,
                    "unable to write (write_offset=%lu+%lu) data to content file: %s\n",
                    write_offset, i, host_path);
            nrf_fclose(fd);
            return -EINVAL;
        }
    }
    nrf_fclose(fd);

    return 0;
}

/*! Get the total size in bytes of a file.
 *  \param[in] path path to the file that gets selected.
 *  \returns size in bytes on success, 0 on failure */
size_t ss_storage_get_file_len(struct ss_list *path) {
    char host_path[MAX_FILE_NAME + 1];
    int rc;
    nrf_FILE *fd;
    long file_size;

    /* TODO: check if the path really points to a file. If the path points
     * to a directory print an error and return a size of 0. (Normally this
     * shouldn't happen because the FCP is always checked before calling
     * this function. */

    rc = gen_abs_host_path(host_path, path, false, "file-len");
    if (rc < 0)
        return 0;

    fd = nrf_fopen(host_path, "r");
    if (!fd) {
        SS_LOGP(SSTORAGE, LERROR, "unable to open content file: %s\n", host_path);
        return 0;
    }

    rc = nrf_fseek(fd, 0, SEEK_END);
    if (rc != 0) {
        SS_LOGP(SSTORAGE, LERROR, "unable to seek requested data in content file: %s\n",
                host_path);
        nrf_fclose(fd);
        return 0;
    }

    file_size = nrf_ftell(fd);
    if (file_size < 0) {
        SS_LOGP(SSTORAGE, LERROR, "unable to tell the size of the file: %s\n", host_path);
        nrf_fclose(fd);
        return 0;
    }

    nrf_fclose(fd);

    /* The files contain ASCII hex digits */
    file_size /= 2;

    return file_size;
}
/*! Delete file or directory in the file system.
 *  \param[in] path path to the file or directory to delete
 *  \returns 0 on success, -EINVAL on failure */
int ss_storage_delete(struct ss_list *path) {
    char host_path_def[MAX_FILE_NAME + 1];
    char host_path_content[MAX_FILE_NAME + 1];
    // char rm_command[10 + MAX_FILE_NAME + 1];
    struct ss_file *file;
    int rc;

    file = ss_get_file_from_path(path);
    if (!file)
        return -EINVAL;

    rc = gen_abs_host_path(host_path_def, path, true, "delete");
    if (rc < 0)
        return -EINVAL;
    rc = gen_abs_host_path(host_path_content, path, false, "delete");
    if (rc < 0)
        return -EINVAL;

    rc = nrf_remove(host_path_def);
    if (rc < 0) {
        SS_LOGP(SSTORAGE, LERROR, "unable to remove definition file: %s\n", host_path_def);
        return -EINVAL;
    }

    // snprintf(rm_command, sizeof(rm_command), "rm -rf %s", host_path_content);
    // rc = system(rm_command);
    // if (rc < 0) {
    // 	SS_LOGP(SSTORAGE, LERROR, "unable to remove content file: %s\n", host_path_content);
    // 	return -EINVAL;
    // }
    return 0;
}

/*! Update definition file in the file system.
 *  \param[in] path path to the file to update
 *  \returns 0 on success, -EINVAL on failure */
int ss_storage_update_def(struct ss_list *path) {
    char host_path[MAX_FILE_NAME + 1];
    int rc;
    nrf_FILE *fd;
    struct ss_file *file;
    size_t i;
    char hex[3];
    size_t fwrite_rc;

    file = ss_get_file_from_path(path);
    if (!file)
        return -EINVAL;

    if (!file->fci) {
        SS_LOGP(SSTORAGE, LERROR, "file (%04x) has no definition (FCP) set -- abort\n",
                file->fid);
        return -EINVAL;
    }

    /* Generate definition file */
    rc = gen_abs_host_path(host_path, path, true, "update-def");
    if (rc < 0)
        return -EINVAL;

    fd = nrf_fopen(host_path, "w");
    if (!fd) {
        SS_LOGP(SSTORAGE, LERROR, "unable to create definition file: %s\n", host_path);
        return -EINVAL;
    }
    for (i = 0; i < file->fci->len; i++) {
        snprintf(hex, sizeof(hex), "%02x", file->fci->data[i]);
        fwrite_rc = nrf_fwrite(hex, sizeof(hex) - 1, 1, fd);
        if (fwrite_rc != 1) {
            SS_LOGP(SSTORAGE, LERROR, "unable to write file definition: %s\n",
                    host_path);
            ss_storage_delete(path);
            nrf_fclose(fd);
            return -EINVAL;
        }
    }
    nrf_fclose(fd);
    return 0;
}

/*! Create a file in the file system.
 *  \param[in] path path to the file that gets selected.
 *  \param[in] file_len length of the file to create (filled with 0xff).
 *  \returns 0 success, -EINVAL on failure */
int ss_storage_create_file(struct ss_list *path, size_t file_len) {
    /*! Note: This function must not be called with pathes that point to
     *  a directory! */

    char host_path[MAX_FILE_NAME + 1];
    int rc;
    nrf_FILE *fd;
    size_t i;

    /* Create definition file */
    rc = ss_storage_update_def(path);
    if (rc < 0)
        return -EINVAL;

    /* Generate (empty) content file */
    rc = gen_abs_host_path(host_path, path, false, "create-file");
    if (rc < 0) {
        ss_storage_delete(path);
        return -EINVAL;
    }
    fd = nrf_fopen(host_path, "w");
    if (!fd) {
        SS_LOGP(SSTORAGE, LERROR, "unable to create content file: %s\n", host_path);
        ss_storage_delete(path);
        return -EINVAL;
    }
    for (i = 0; i < file_len * 2; i++) {
        if (nrf_fputc('f', fd) != 'f') {
            SS_LOGP(SSTORAGE, LERROR, "unable to prefill content file: %s\n",
                    host_path);
            ss_storage_delete(path);
            nrf_fclose(fd);
            return -EINVAL;
        }
    }
    nrf_fclose(fd);

    return 0;
}

/*! Create a directory in the file system.
 *  \param[in] path path to the directory to create
 *  \returns 0 on success, -EINVAL on failure */
int ss_storage_create_dir(struct ss_list *path) {
    /*! Note: This function must not be called with pathes that point to
     *  a directory! */

    char host_path[MAX_FILE_NAME + 1];
    int rc;

    /* Create definition file */
    rc = ss_storage_update_def(path);
    if (rc < 0)
        return -EINVAL;

    /* Create directory */
    rc = gen_abs_host_path(host_path, path, false, "create-dir");
    if (rc < 0) {
        ss_storage_delete(path);
        return -EINVAL;
    }

    if (nrf_access(host_path, W_OK) == 0)  // TODO
        return 0;
    rc = nrf_mkdir(host_path, 0700);
    if (rc < 0) {
        SS_LOGP(SSTORAGE, LERROR, "unable to create directory: %s\n", host_path);
        ss_storage_delete(path);
        return -EINVAL;
    }

    return 0;
}

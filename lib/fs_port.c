#include <onomondo/softsim/fs_port.h>
#include <onomondo/softsim/utils.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "profile.h"
#include "impl_fs_port.h"


// #define PROFILE_PORT_IMPL
// #define PROFILE_PUTC
#ifdef PROFILE_PORT_IMPL

#ifdef LOG_FUNC_NAME_PREFIX_DBG
#define LOG_PORT_CALL() LOG_DBG("%s Called", __func__)
#define LOG_PORT_CALL_LONG(x, fmt, ...) LOG_DBG("%s rc: %ld - " fmt, __func__, x, __VA_ARGS__)
#define LOG_PORT_CALL_INT(x, fmt, ...) LOG_DBG("%s rc: %d - " fmt, __func__, x, __VA_ARGS__)
#define LOG_PORT_CALL_PTR(x, fmt, ...) LOG_DBG("%s rc: %p - " fmt, __func__, x, __VA_ARGS__)
#else
#define LOG_PORT_CALL() LOG_DBG("Called")
#define LOG_PORT_CALL_LONG(x, fmt, ...) LOG_DBG("rc: %ld - " fmt, x, __VA_ARGS__)
#define LOG_PORT_CALL_INT(x, fmt, ...) LOG_DBG("rc: %d - " fmt, x, __VA_ARGS__)
#define LOG_PORT_CALL_PTR(x, fmt, ...) LOG_DBG("rc: %p - " fmt, x, __VA_ARGS__)
#endif
#else
#define LOG_PORT_CALL()
#define LOG_PORT_CALL_LONG(x, fmt, ...)
#define LOG_PORT_CALL_INT(x, fmt, ...)
#define LOG_PORT_CALL_PTR(x, fmt, ...)
#endif

LOG_MODULE_REGISTER(softsim_fs_port, CONFIG_SOFTSIM_LOG_LEVEL);

int init_fs() {
    LOG_PORT_CALL();
    return impl_init_fs();
}

/*
 * This will only be called when softsim is deinitialized.
 * I.e. when the modem goes to cfun=0 or cfun=4
 * */
int deinit_fs() {
    LOG_PORT_CALL();
    return impl_deinit_fs();
}

/**
 * @brief Implements a version of standard C fopen.
 * @param path full path.
 * @param mode 
 * @return pointer to a file handle
 */
port_FILE port_fopen(char *path, char *mode) {

    port_FILE rc = impl_port_fopen(path, mode);
    LOG_PORT_CALL_PTR(rc, "path: %s mode: %s", path, mode);
    return rc;
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
    size_t rc = impl_port_fread(ptr, size, nmemb, fp);
    LOG_PORT_CALL_INT((int)rc, "size: %d, nmemb: %d, fp: %p", (int)size, (int)nmemb, fp);
    return rc;
}

int port_fclose(port_FILE fp) {
    int rc = impl_port_fclose(fp);
    LOG_PORT_CALL_INT(rc, "fp: %p", fp);
    return rc;
}

int port_fseek(port_FILE fp, long offset, int whence) {
    int rc = impl_port_fseek(fp, offset, whence);
    LOG_PORT_CALL_INT(rc, "fp: %p, offset: %ld, whence: %d", fp, offset, whence);
    return rc;
}

long port_ftell(port_FILE fp) {
    long rc = impl_port_ftell(fp);
    LOG_PORT_CALL_LONG(rc, "fp: %p", fp);
    return rc;
}

int port_fputc(int c, port_FILE fp) {
    int rc = impl_port_fputc(c, fp);
#ifdef PROFILE_PUTC
    LOG_PORT_CALL_INT(rc, "fp: %p, c: %d", fp, c);
#endif
    return rc;
}

int port_access(const char *path, int amode) {
    int rc = impl_port_access(path, amode);
    LOG_PORT_CALL_INT(rc, "path: %s, amode: %d", path, amode);
    return rc;
}

int port_mkdir(const char *path, int _) {
    int rc = impl_port_mkdir(path, _);
    LOG_PORT_CALL_INT(rc, "path: %s", path);
    return rc;
}  

int port_remove(const char *path) {
    int rc = impl_port_remove(path);
    LOG_PORT_CALL_INT(rc, "path: %s", path);
    return rc;
}

// very unlike to be invoked tbh
int port_rmdir(const char *path) {
    int rc = impl_port_rmdir(path);
    LOG_PORT_CALL_INT(rc, "path: %s", path);
    return rc;
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
    size_t rc = impl_port_fwrite(ptr, size, count, fp);
    LOG_PORT_CALL_INT((int)rc, "size: %d, count: %d, fp: %p", (int)size, (int)count, fp);
    return rc;
}

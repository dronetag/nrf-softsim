#include <onomondo/softsim/fs_port.h>
#include <onomondo/softsim/utils.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "profile.h"
#include "impl_fs_port.h"


LOG_MODULE_REGISTER(softsim_fs_port, CONFIG_SOFTSIM_LOG_LEVEL);

int init_fs() {
    return impl_init_fs();
}

/*
 * This will only be called when softsim is deinitialized.
 * I.e. when the modem goes to cfun=0 or cfun=4
 * */
int deinit_fs() {
    return impl_deinit_fs();
}

/**
 * @brief Implements a version of standard C fopen.
 * @param path full path.
 * @param mode 
 * @return pointer to a file handle
 */
port_FILE port_fopen(char *path, char *mode) {
    return impl_port_fopen(path, mode);
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
    return impl_port_fread(ptr, size, nmemb, fp);
}

int port_fclose(port_FILE fp) {
    return impl_port_fclose(fp);
}

int port_fseek(port_FILE fp, long offset, int whence) {
    return impl_port_fseek(fp, offset, whence);
}

long port_ftell(port_FILE fp) {
    return impl_port_ftell(fp);
}

int port_fputc(int c, port_FILE fp) {
    return impl_port_fputc(c, fp);
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
    return impl_port_fwrite(ptr, size, count, fp);
}

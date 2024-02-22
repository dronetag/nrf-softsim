#ifndef IMPL_FS_impl_port_H
#define IMPL_FS_impl_port_H

#include <stddef.h>
struct ss_profile;
typedef void *impl_port_FILE;
size_t impl_port_fread(void *ptr, size_t size, size_t nmemb, impl_port_FILE fp);
int impl_port_fclose(impl_port_FILE);
impl_port_FILE impl_port_fopen(char *path, char *mode);
int impl_port_fseek(impl_port_FILE fp, long offset, int whence);
long impl_port_ftell(impl_port_FILE fp);
int impl_port_fputc(int c, impl_port_FILE);
int impl_port_access(const char *path, int amode);
size_t impl_port_fwrite(const void *prt, size_t size, size_t count, impl_port_FILE f);
int impl_port_mkdir(const char *, int);
int impl_port_remove(const char *);
int impl_port_rmdir(const char *);
int impl_init_fs();
int impl_deinit_fs();
int impl_port_check_provisioned();
int impl_port_provision(struct ss_profile *profile);
int impl_port_size(impl_port_FILE);
int impl_port_stat(const char *);

#endif /* IMPL_FS_impl_port_H */

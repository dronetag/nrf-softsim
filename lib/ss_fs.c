#include <onomondo/softsim/utils.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#include "../littlefs/scripts/ss_static_files.h"
#include "provision.h"
#include "profile.h"
#include "impl_fs_port.h"

// LOG_MODULE_REGISTER(softsim_fs, 4);
LOG_MODULE_REGISTER(softsim_fs, CONFIG_SOFTSIM_LOG_LEVEL);
#define DEBUG_PROFILE_PROVISION



//#define DEBUG_FS_CALLS

#ifdef DEBUG_FS_CALLS
#define FS_LOG_DBG(...) LOG_DBG(__VA_ARGS__)
#else
#define FS_LOG_DBG(...)
#endif

#define DIR_ID (1UL)

#define IMSI_PATH "/3f00/7ff0/6f07"
#define ICCID_PATH "/3f00/2fe2"
#define A001_PATH "/3f00/a001"
#define A004_PATH "/3f00/a004"

int impl_port_provision(struct ss_profile *profile);
int impl_port_check_provisioned(void);

struct ss_fs_ctx {
    bool is_initialized;
};


struct ss_fs_file {
    struct fs_file_t file;
};

static struct ss_fs_ctx fs_ctx;
static int ss_fs_ctx_init(struct ss_fs_ctx *fs_ctx) {
    fs_ctx->is_initialized = false;
    return 0;
}

static int ss_fs_file_init(struct ss_fs_file *f) {
    fs_file_t_init(&f->file);
    return 0;
}


static uint8_t default_imsi[] = {0x08, 0x09, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10};

static int ss_fs_inline_read(const char *path, void *ptr, size_t size)
{
    int rc;
    struct fs_file_t f;
    fs_file_t_init(&f);
    rc = fs_open(&f, path, FS_O_READ);
    if(rc) {
        return rc;
    }

    rc = fs_read(&f, ptr, size);
    if(rc < 0) {
        return rc;
    }
    size_t read_len = rc;

    rc = fs_close(&f);
    if(rc) {
        return rc;
    }
    return read_len;
}

static int ss_fs_inline_mkdir(const char *path)
{
    int rc;
    char mkdir_path[CONFIG_SOFTSIM_FS_PATH_LEN];
    strncpy(mkdir_path, path, sizeof(mkdir_path));
    /* Create all folders in the path */
    int part = 0;
    bool last = false;
    char *pathPartStart = mkdir_path;
    char *pathPartEnd = NULL;
    while(true) {
        pathPartEnd = strchr(pathPartStart, '/');
        /* Reached the end */
        if(pathPartEnd == NULL) {
            pathPartEnd = pathPartStart + strlen(pathPartStart);
            last = true;
        }
        part++;

        /* Skip partition creation of first two '/' example: /storage/ */
        if(part > 2) {

            /* Replace / with end of string */
            *pathPartEnd = '\0';
            LOG_DBG("Creating folder %s", mkdir_path);
            struct fs_dirent ent;
            /* To get rid off NCS error when creating already existing folder */
            rc = fs_stat(mkdir_path, &ent);
            if(rc == -ENOENT) {
                rc = fs_mkdir(mkdir_path);
                if(rc && rc != -EEXIST) {
                    return rc;
                }
            }
            if(last) {
                break;
            }
            /* After creation return it back */
            *pathPartEnd = '/';
        }

        pathPartStart = pathPartEnd+1;
    }
    return 0;
}

static int ss_fs_inline_write(const char *path, const void *ptr, size_t size)
{
    int rc;
    struct fs_file_t f;
    fs_file_t_init(&f);
    rc = fs_open(&f, path, FS_O_WRITE | FS_O_CREATE);
    if(rc) {
        LOG_ERR("Inline write failed to open file: %s", path);
        return rc;
    }

    rc = fs_write(&f, ptr, size);
    if(rc < 0) {
        return rc;
    }
    size_t write_len = rc;
    /* TODO: check if written all ? */

    rc = fs_close(&f);
    if(rc) {
        return rc;
    }
    return write_len;
}

int impl_init_fs() {
  if (fs_ctx.is_initialized) return 0;  // already initialized
  int rc = ss_fs_ctx_init(&fs_ctx);
  if(rc) {
      LOG_ERR("Failed to initialize context");
      return rc;
  }

  fs_ctx.is_initialized = true;

#ifdef CONFIG_SOFTSIM_FS_BACKUP
  if(impl_port_check_provisioned() == 0) {
      struct ss_profile backup_profile;
      int rc = 0;
      /* TODO: Maybe check all ? */
      rc = ss_fs_inline_read(CONFIG_SOFTSIM_FS_BACKEND_PREFIX IMSI_PATH, backup_profile.IMSI, IMSI_LEN);
      if(rc > 0) {
          /* Backup exists do recovery and reinit */
          LOG_INF("Restoring SoftSIM 1/4");
          rc = ss_fs_inline_read(CONFIG_SOFTSIM_FS_BACKEND_PREFIX IMSI_PATH, backup_profile.IMSI, IMSI_LEN);
          if(rc < 0) {
              goto out_err;
          }

          LOG_INF("Restoring SoftSIM 2/4");
          rc = ss_fs_inline_read(CONFIG_SOFTSIM_FS_BACKEND_PREFIX ICCID_PATH, backup_profile.ICCID, ICCID_LEN);
          if(rc < 0) {
              goto out_err;
          }

          LOG_INF("Restoring SoftSIM 3/4");
          rc = ss_fs_inline_read(CONFIG_SOFTSIM_FS_BACKEND_PREFIX A001_PATH, backup_profile.A001, sizeof(backup_profile.A001));
          if(rc < 0) {
              goto out_err;
          }

          LOG_INF("Restoring SoftSIM 4/4");
          rc = ss_fs_inline_read(CONFIG_SOFTSIM_FS_BACKEND_PREFIX A004_PATH, backup_profile.A004, sizeof(backup_profile.A004));
          if(rc < 0) {
              goto out_err;
          }
          rc = impl_port_provision(&backup_profile);
          if(rc) {
              LOG_ERR("Profile backup recovery failed: %d", rc);
          }
      }
  }
out_err:
#endif


  return 0;
}

/*
 * This will only be called when softsim is deinitialized.
 * I.e. when the modem goes to cfun=0 or cfun=4
 * */
int impl_deinit_fs() {
  /* TODO: Do we want to close all files just to be sure ? */
  fs_ctx.is_initialized = false;
  return 0;
}

/**
 * @brief Implements a version of standard C fopen.
 * @param path full path.
 * @param mode 
 * @return pointer to a file handle
 */
impl_port_FILE impl_port_fopen(char *path, char *mode) {
  int rc = 0;
  struct ss_fs_file *f = SS_ALLOC(struct ss_fs_file);
  if(!f) {
      return NULL;
  }

  /* Set errno ? */
  rc = ss_fs_file_init(f);
  if(rc) {
    return NULL;
  }

  const size_t prefLen = sizeof(CONFIG_SOFTSIM_FS_BACKEND_PREFIX)-1;
  char file_path[CONFIG_SOFTSIM_FS_PATH_LEN] = CONFIG_SOFTSIM_FS_BACKEND_PREFIX;
  strncpy(file_path+prefLen, path, sizeof(file_path)-prefLen);

  FS_LOG_DBG("open path: %s, mode: %s", file_path, mode);

  /* TODO: Parse mode */
  rc = fs_open(&f->file, file_path, FS_O_RDWR | FS_O_CREATE);
  if(rc) {
    LOG_ERR("Failed to open file: %s", file_path);
    SS_FREE(f);
    return NULL;
  }
  return f;
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
size_t impl_port_fread(void *ptr, size_t size, size_t nmemb, impl_port_FILE fp) {
  if (nmemb == 0 || size == 0) {
    return 0;
  }
  struct ss_fs_file *f = fp;
  int rc = fs_read(&f->file, ptr, size*nmemb);
  if(rc < 0) {
    LOG_ERR("Failed to read file");
    return 0;
  }
  /* Return number of elements read */
  return rc/size;
}

int impl_port_fclose(impl_port_FILE fp) {
  struct ss_fs_file *f = fp;
  FS_LOG_DBG("close file");
  int rc = fs_close(&f->file);
  if(rc) {
    LOG_ERR("Failed to close file");
  }
  SS_FREE(f);
  return 0;
}

int impl_port_fseek(impl_port_FILE fp, long offset, int whence) {
  struct ss_fs_file *f = fp;
  int rc = fs_seek(&f->file, offset, whence);
  if(rc) {
    LOG_ERR("Seek failed");
  }
  return rc;
}
long impl_port_ftell(impl_port_FILE fp) {
  struct ss_fs_file *f = fp;
  int rc = fs_tell(&f->file);
  if(rc < 0) {
    LOG_ERR("Seek failed");
  }
  return rc;
}

int impl_port_fputc(int c, impl_port_FILE fp) {
  size_t written = impl_port_fwrite(&c, 1, 1, fp);

  if (written == 1) {
    return c;
  }
  return -1;
}

// TODO -> safe to omit for now. Internally SoftSIM will verify that a
// directory exists after creation.
int impl_port_access(const char *path, int amode) {
  return 0;
}

int impl_port_mkdir(const char *path, int _) {
  int rc = fs_mkdir(path);
  if(rc) {
    LOG_ERR("Failed to create directory path: %s", path);
  }
  return rc;
}  

int impl_port_remove(const char *path) {
  int rc = fs_unlink(path);
  if(rc) {
    LOG_ERR("Failed to remove path: %s", path);
  }
  return 0;
}

int impl_port_size(impl_port_FILE fp)
{
  struct ss_fs_file *f = fp;
  long offset = fs_tell(&f->file);
  if(offset < 0) {
    return 0;
  }
  int rc = fs_seek(&f->file, 0, FS_SEEK_END);
  if(rc) {
    return 0;
  }
  long size = fs_tell(&f->file);
  if(size < 0) {
    return 0;
  }
  rc = fs_seek(&f->file, offset, FS_SEEK_SET);
  if(rc) {
    return 0;
  }
  return size;
}

int impl_port_stat(const char *path)
{
  uint32_t start = k_uptime_get_32();
  struct fs_dirent ent;
  const size_t prefLen = sizeof(CONFIG_SOFTSIM_FS_BACKEND_PREFIX)-1;
  char file_path[CONFIG_SOFTSIM_FS_PATH_LEN] = CONFIG_SOFTSIM_FS_BACKEND_PREFIX;
  strncpy(file_path+prefLen, path, sizeof(file_path)-prefLen);


  int rc = fs_stat(file_path, &ent);
  uint32_t end = k_uptime_get_32();
  LOG_WRN("Stat duration: %u", end-start);
  if(rc) {
    return rc;
  }
  if(ent.type != FS_DIR_ENTRY_FILE) {
    return -ENOENT;
  }
  return ent.size;
}

// very unlike to be invoked tbh
int impl_port_rmdir(const char *path) {
  int rc = fs_unlink(path);
  if(rc) {
    LOG_ERR("Failed to remove directory: %s", path);
  }
  return 0;
}


int impl_port_check_provisioned() {
  uint8_t buffer[IMSI_LEN];
  int rc = 0;
  rc = ss_fs_inline_read(CONFIG_SOFTSIM_FS_BACKEND_PREFIX IMSI_PATH, buffer, IMSI_LEN);
  if(rc < 0) {
    /* Return 0 on error to correctly signal that the port is not provisioned */
    return 0;
  }
  /* TODO: check if read all ? */

  // IMSI read is still the default one from the template
  if (memcmp(buffer, default_imsi, IMSI_LEN) == 0) {
    return 0;
  }

  return 1;
}

__weak void softsim_watchdog_feed()
{
}

/**
 * @brief Provision the SoftSIM with the given profile
 *
 *
 * @param profile ptr to the profile
 * @param len Len of profile. 332 otherwise invalid.
 */
int impl_port_provision(struct ss_profile *profile) {
  int rc = impl_init_fs();
  if (rc) {
    LOG_ERR("Failed to init FS");
  }

#ifdef CONFIG_SOFTSIM_TEMPLATE_GENERATION_CODE
  for(int i = 0; i < onomondo_sf_dirs_len; i++) {
    const size_t prefLen = sizeof(CONFIG_SOFTSIM_FS_BACKEND_PREFIX)-1;
    char dir_path[CONFIG_SOFTSIM_FS_PATH_LEN] = CONFIG_SOFTSIM_FS_BACKEND_PREFIX;
    strncpy(dir_path+prefLen, onomondo_sf_dirs[i].name, sizeof(dir_path)-prefLen);

    rc = ss_fs_inline_mkdir(dir_path);
    if(rc < 0) {
          LOG_ERR("Failed to provision directory: %s - rc: %d", onomondo_sf_dirs[i].name, rc);
          return rc;
    }
  }
  for(int i = 0; i < onomondo_sf_files_len; i++) {
    const size_t prefLen = sizeof(CONFIG_SOFTSIM_FS_BACKEND_PREFIX)-1;
    char file_path[CONFIG_SOFTSIM_FS_PATH_LEN] = CONFIG_SOFTSIM_FS_BACKEND_PREFIX;
    strncpy(file_path+prefLen, onomondo_sf_files[i].name, sizeof(file_path)-prefLen);

    rc = ss_fs_inline_write(file_path, onomondo_sf_files[i].data, onomondo_sf_files[i].size);
    if(rc < 0) {
      LOG_ERR("Failed to provision file: %s - rc: %d", file_path, rc);
      return rc;
    }
    softsim_watchdog_feed();
  }
#endif

#ifdef DEBUG_PROFILE_PROVISION
  LOG_HEXDUMP_DBG(profile->IMSI, IMSI_LEN, IMSI_PATH);
  LOG_HEXDUMP_DBG(profile->ICCID, ICCID_LEN, ICCID_PATH);
  LOG_HEXDUMP_DBG(profile->A001, sizeof(profile->A001), A001_PATH);
  LOG_HEXDUMP_DBG(profile->A004, sizeof(profile->A004), A004_PATH);
#endif

#ifdef CONFIG_SOFTSIM_FS_BACKUP
  /* TODO: Remount as write mount. The read only mount should be almost impossible to corrupt */
  LOG_INF("Provisioning backup SoftSIM 1/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKUP_PREFIX IMSI_PATH, profile->IMSI, IMSI_LEN);
  if(rc < 0) {
    goto out_err;
  }

  LOG_INF("Provisioning backup SoftSIM 2/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKUP_PREFIX ICCID_PATH, profile->ICCID, ICCID_LEN);
  if(rc < 0) {
    goto out_err;
  }

  LOG_INF("Provisioning backup SoftSIM 3/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKUP_PREFIX A001_PATH, profile->A001, sizeof(profile->A001));
  if(rc < 0) {
    goto out_err;
  }

  LOG_INF("Provisioning backup SoftSIM 4/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKUP_PREFIX A004_PATH, profile->A004, sizeof(profile->A004));
  if(rc < 0) {
    goto out_err;
  }
#endif


  LOG_INF("Provisioning SoftSIM 1/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKEND_PREFIX IMSI_PATH, profile->IMSI, IMSI_LEN);
  if(rc < 0) {
    goto out_err;
  }

  LOG_INF("Provisioning SoftSIM 2/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKEND_PREFIX ICCID_PATH, profile->ICCID, ICCID_LEN);
  if(rc < 0) {
    goto out_err;
  }

  LOG_INF("Provisioning SoftSIM 3/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKEND_PREFIX A001_PATH, profile->A001, sizeof(profile->A001));
  if(rc < 0) {
    goto out_err;
  }

  LOG_INF("Provisioning SoftSIM 4/4");
  rc = ss_fs_inline_write(CONFIG_SOFTSIM_FS_BACKEND_PREFIX A004_PATH, profile->A004, sizeof(profile->A004));
  if(rc < 0) {
    goto out_err;
  }

  /* TODO: Create a backup in WriteOnce partition. */
  LOG_INF("SoftSIM provisioned");
  return 0;

out_err:
  LOG_ERR("SoftSIM provisioning failed");
  return rc;
}

size_t impl_port_fwrite(const void *ptr, size_t size, size_t count, impl_port_FILE fp) {
  if (size == 0 || count == 0) {
    return 0;
  }
  struct ss_fs_file *f = fp;
  int rc = fs_write(&f->file, ptr, size*count);
  if(rc < 0) {
    LOG_ERR("Failed to write to file");
    return 0;
  }
  return rc/size;
}

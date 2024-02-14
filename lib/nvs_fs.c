#include <onomondo/softsim/list.h>
#include <onomondo/softsim/utils.h>
#include <onomondo/softsim/log.h>
#include <onomondo/softsim/mem.h>
#include <stdlib.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

#include "../littlefs/scripts/ss_static_files.h"
#include "f_cache.h"
#include "provision.h"
#include "profile.h"
#include "impl_fs_port.h"

LOG_MODULE_REGISTER(softsim_nvs, CONFIG_SOFTSIM_LOG_LEVEL);

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#if DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, softsim_partition)
#define NVS_PARTITION_ID DT_FIXED_PARTITION_ID(DT_PROP_BY_PHANDLE(ZEPHYR_USER_NODE, softsim_partition))
#elif DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, softsim_partition_name)
#define NVS_PARTITION_ID FIXED_PARTITION_ID(DT_STRING_TOKEN(ZEPHYR_USER_NODE, softsim_partition_name))
#else
#define NVS_PARTITION_ID FIXED_PARTITION_ID(nvs_storage)
#endif

#if DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, softsim_backup_partition)
#define NVS_BACKUP_PARTITION_ID DT_FIXED_PARTITION_ID(DT_PROP_BY_PHANDLE(ZEPHYR_USER_NODE, softsim_partition))
#elif DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, softsim_backup_partition_name)
#define NVS_BACKUP_PARTITION_ID FIXED_PARTITION_ID(DT_STRING_TOKEN(ZEPHYR_USER_NODE, softsim_partition_name))
#endif

static struct nvs_fs fs;
#ifdef NVS_BACKUP_PARTITION_ID
static struct nvs_fs fs_backup;
#endif
static struct cache_ctx fs_cache;


#define DIR_ID (1UL)

#define IMSI_PATH "/3f00/7ff0/6f07"
#define ICCID_PATH "/3f00/2fe2"
#define A001_PATH "/3f00/a001"
#define A004_PATH "/3f00/a004"

#define IMSI_BACKUP_ID 1000
#define ICCID_BACKUP_ID 1001
#define A001_BACKUP_ID 1002
#define A004_BACKUP_ID 1003

#ifndef SEEK_SET
#define SEEK_SET 0 /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1 /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END 2 /* set file offset to EOF plus offset */
#endif

// internal functions
static int port_provision_inner(struct ss_profile *profile, bool restore);

uint8_t fs_is_initialized = 0;
uint8_t nvs_is_initialized = 0;

int nvs_fs_cache_entry_write(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len)
{
    ARG_UNUSED(ctx);
    int rc = nvs_write(&fs, entry->key, buffer, len);
    if(rc < 0) {
        return rc;
    }
    return len;
}

int nvs_fs_cache_entry_read(struct cache_ctx *ctx, struct cache_entry *entry, void *buffer, size_t len)
{
    ARG_UNUSED(ctx);
    return nvs_read(&fs, entry->key, buffer, len);
}

uint16_t nvs_fs_cache_entry_length(struct cache_ctx *ctx, struct cache_entry *entry)
{
    return nvs_read(&fs, entry->key, NULL, 0);
}

int nvs_fs_cache_entry_remove(struct cache_ctx *ctx, struct cache_entry *entry)
{
    return nvs_delete(&fs, entry->key);
}

struct cache_strorage_funcs fs_cache_storage_funcs = {
    .read = nvs_fs_cache_entry_read,
    .write = nvs_fs_cache_entry_write,
    .length = nvs_fs_cache_entry_length,
    .remove = nvs_fs_cache_entry_remove
};

int load_dirs() {
    uint8_t *data = NULL;
    size_t len = 0;
    f_cache_init(&fs_cache, &fs_cache_storage_funcs);

    int rc = nvs_read(&fs, DIR_ID, NULL, 0);
    if(rc < 0) {
        LOG_ERR("NVS Read failed to load dir_id: %d", rc);
        goto out;
    }

    len = rc;

    /*************************
   * Read DIR_ENTRY from NVS
   * This is used to construct a linked list that
   * serves as a cache and lookup table for the filesystem
   */
    if (!data && rc) {
        data = SS_ALLOC_N(len * sizeof(uint8_t));
        rc = nvs_read(&fs, DIR_ID, data, len);
        __ASSERT_NO_MSG(rc == len);
    }

    /* Thanks to this dir blob the NVS file system is keeping
     * all records mapping path -> key in side the cache */
    generate_dir_table_from_blob(&fs_cache, data, len);

    if (f_cache_empty(&fs_cache)) goto out;
out:
    SS_FREE(data);
    return f_cache_empty(&fs_cache);
}

int init_nvs_from_id(struct nvs_fs *nvs, int partition_id, bool erase)
{
    const struct flash_area *nvs_area;
    int rc = flash_area_open(NVS_PARTITION_ID, &nvs_area);
    if(rc) {
        LOG_ERR("Failed to open flash area: %d", rc);
        return -1;
    }
    struct flash_sector hw_flash_sector;
    uint32_t sector_cnt = 1;

    rc = flash_area_get_sectors(NVS_PARTITION_ID, &sector_cnt,
                                &hw_flash_sector);
    if(rc && rc != -ENOMEM) {
        LOG_ERR("Failed to retrieve information about sectors: %d", rc);
        return -1;
    }
    if(erase) {
        LOG_INF("NVS Softsim storage erasing...");
        rc = flash_area_erase(nvs_area, 0, nvs_area->fa_size);
        if(rc) {
            LOG_ERR("Failed to erase area: %d", rc);
            return rc;
        }
    }

    fs.flash_device = nvs_area->fa_dev;
    fs.sector_size = hw_flash_sector.fs_size;
    fs.sector_count = nvs_area->fa_size / hw_flash_sector.fs_size;
    fs.offset = nvs_area->fa_off;

    rc = nvs_mount(&fs);
    if (rc) {
        LOG_ERR("failed to mount nvs\n");
        return -1;
    }
    return 0;
}

int init_nvs(bool erase) {
    int rc;
    if (nvs_is_initialized && !erase) return 0;  // already initialized
    rc = init_nvs_from_id(&fs, NVS_PARTITION_ID, erase);
    if(rc) {
        return rc;
    }
#ifdef NVS_BACKUP_PARTITION_ID
    rc = init_nvs_from_id(&fs_backup, NVS_BACKUP_PARTITION_ID, false);
    if(rc) {
        return rc;
    }
#endif
    nvs_is_initialized++;
    return 0;
}

int impl_init_fs() {
  if (fs_is_initialized) return 0;  // already initialized
  int rc = 0;
  rc = init_nvs(false);
  if(rc) {
    return -1;
  }

  rc = load_dirs();
  if(rc) {
    /* TODO: Somehow monitor the XSIM notification to detect FS Corruption */
#ifdef CONFIG_SOFTSIM_TEMPLATE_GENERATION_CODE
    /* Load dirs is corrupted check if we have the backup to restore the FS */
#ifdef NVS_BACKUP_PARTITION_ID
    struct ss_profile backup_profile;
    int rc = 0;
    /* TODO: Maybe check all ? */
    rc = nvs_read(&fs_backup, IMSI_BACKUP_ID, backup_profile.IMSI, IMSI_LEN);
    if(rc > 0) {
      /* Backup exists do recovery and reinit */
      LOG_INF("Restoring SoftSIM 1/4");
      rc = nvs_read(&fs_backup, IMSI_BACKUP_ID, backup_profile.IMSI, IMSI_LEN);
      if(rc < 0) {
          goto out_err;
      }

      LOG_INF("Restoring SoftSIM 2/4");
      rc = nvs_read(&fs_backup, ICCID_BACKUP_ID, backup_profile.ICCID, ICCID_LEN);
      if(rc < 0) {
          goto out_err;
      }

      LOG_INF("Restoring SoftSIM 3/4");
      rc = nvs_read(&fs_backup, A001_BACKUP_ID, backup_profile.A001, sizeof(backup_profile.A001));
      if(rc < 0) {
          goto out_err;
      }

      LOG_INF("Restoring SoftSIM 4/4");
      rc = nvs_read(&fs_backup, A004_BACKUP_ID, backup_profile.A004, sizeof(backup_profile.A004));
      if(rc < 0) {
          goto out_err;
      }
      rc = port_provision_inner(&backup_profile, true);
      if(rc) {
          LOG_ERR("Profile backup recovery failed: %d", rc);
      }
    }
  out_err:
#endif
#else
    return -1;
#endif
  }
  fs_is_initialized++;
  return rc;
}

int impl_deinit_fs() {
  // TODO: check if DIR entry is still valid
  // if not recreate and write.
  // Will NVS only commit if there are changes? If so, we can just recreate
  // and let NVS do the compare.
  f_cache_close(&fs_cache);
  fs_is_initialized = 0;

  return 0;
}

/**
 * @brief Implements a version of standard C fopen.
 * @param path Full path.
 * @param mode Currently ignorred.
 * @return Pointer to a
 * "file" represented by a struct cache_entry internally.
 */
impl_port_FILE impl_port_fopen(char *path, char *mode) {
  struct cache_entry *entry = NULL;
  int rc = 0;
  entry = f_cache_find_by_name(path, &fs_cache);
  if (!entry) {
    LOG_DBG("Entry not found in the cache: %s", path);
    return NULL;
  }
  rc = f_cache_fopen(&fs_cache, entry);
  if(rc) {
    LOG_ERR("Failed to open cache entry: %s", path);
    return NULL;
  }
  return (void *)entry;
}

/**
 * @brief Implements a version of standard C fread. Internally it will use a
 * cache.
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
  struct cache_entry *entry = (struct cache_entry *)fp;
  return f_cache_fread(&fs_cache, entry, ptr, size, nmemb);
}


int impl_port_fclose(impl_port_FILE fp) {
  struct cache_entry *entry = (struct cache_entry *)fp;
  return f_cache_fclose(&fs_cache, entry);
}

int impl_port_fseek(impl_port_FILE fp, long offset, int whence) {
  struct cache_entry *entry = (struct cache_entry *)fp;
  return f_cache_fseek(&fs_cache, entry, offset, whence);
}

long impl_port_ftell(impl_port_FILE fp) {
  struct cache_entry *entry = (struct cache_entry *)fp;
  return f_cache_ftell(&fs_cache, entry);
}

int impl_port_fputc(int c, impl_port_FILE fp) {
  struct cache_entry *entry = (struct cache_entry *)fp;
  return f_cache_putc(&fs_cache, entry, c);
}
int impl_port_access(const char *path, int amode) {
  ARG_UNUSED(path);
  ARG_UNUSED(amode);
  return 0;
}  // TODO -> safe to omit for now. Internally SoftSIM will verify that a
   // directory exists after creation. Easier to guarentee since it isn't a
   // 'thing'
int impl_port_mkdir(const char *_, int __) {
  ARG_UNUSED(_);
  ARG_UNUSED(__);
  return 0;
}  // don't care. We don't really obey directories (creating file
   // 'test/a/b/c.def) implicitly creates the directories

int impl_port_remove(const char *path) {
  struct cache_entry *entry = f_cache_find_by_name(path, &fs_cache);
  return f_cache_remove(&fs_cache, entry);
}

// list for each { is_name_partial_match? {remove port_remove(cursor->name)} }
int impl_port_rmdir(const char *_) {
  ARG_UNUSED(_);
  return 0;
}  // todo. Remove all entries with directory match.

static uint8_t default_imsi[] = {0x08, 0x09, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10};

int impl_port_check_provisioned() {
  int ret;
  uint8_t buffer[IMSI_LEN] = {0};
  struct cache_entry *entry = (struct cache_entry *)f_cache_find_by_name(IMSI_PATH, &fs_cache);
  if(entry == NULL) {
    return 0;
  }

  ret = nvs_read(&fs, entry->key, buffer, IMSI_LEN);
  if (ret < 0) {
    return 0;
  }

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
  return port_provision_inner(profile, false);
}

static int port_provision_inner(struct ss_profile *profile, bool restore)
{
  bool erase_during_provisioning = false;
#ifdef CONFIG_SOFTSIM_TEMPLATE_GENERATION_CODE
  erase_during_provisioning = true;
#endif
  int rc = init_nvs(erase_during_provisioning);
  if (rc) {
    LOG_ERR("Failed to init NVS");
  }

#ifdef CONFIG_SOFTSIM_TEMPLATE_GENERATION_CODE
  size_t provDirsLen = onomondo_sf_files_names_len + onomondo_sf_files_len*3;
  uint8_t *provDirs = SS_ALLOC_N(provDirsLen);
  if(provDirs == NULL) {
    return -ENOMEM;
  }
  uint8_t *provDirsPtr = provDirs;

  for(int i = 0; i < onomondo_sf_files_len; i++) {
    rc = nvs_write(&fs, onomondo_sf_files[i].nvs_key, onomondo_sf_files[i].data, onomondo_sf_files[i].size);
    /* Zero returned when value already written */
    if (rc != 0 && rc != onomondo_sf_files[i].size) {
      LOG_ERR("Failed to provision file: %s - rc: %d", onomondo_sf_files[i].name, rc);
      goto out_err;
    }
    uint8_t fileNameLen = strlen(onomondo_sf_files[i].name);
    if((provDirsPtr-provDirs) + 3 + fileNameLen >= provDirsLen) {
        break;
    }
    /* Store length of the filename */
    provDirsPtr[0] = fileNameLen;
    /* Store associated key */
    provDirsPtr[1] = (onomondo_sf_files[i].nvs_key >> 8) & 0xFF;
    provDirsPtr[2] = (onomondo_sf_files[i].nvs_key) & 0xFF;
    provDirsPtr += 3;
    /* Store filename */
    memcpy(provDirsPtr, onomondo_sf_files[i].name, fileNameLen);
    provDirsPtr += fileNameLen;
    softsim_watchdog_feed();
  }

  ssize_t dirs_len = provDirsPtr-provDirs;
  /* Store the DIR_ID Entry */
  rc = nvs_write(&fs, DIR_ID, provDirs, dirs_len);
  SS_FREE(provDirs);
  /* Zero returned when value already written */
  if(rc != 0 && rc != dirs_len) {
    LOG_ERR("Failed to generate DIRS: %d", rc);
    return -1;
  }

  /* Try to load the dirs after generation */
  rc = load_dirs();
  if(rc) {
    LOG_ERR("Failed to load DIRS even after dirs generation: %d", rc);
    return -1;
  }
#ifdef NVS_BACKUP_PARTITION_ID
  if(!restore) {
      LOG_INF("Provisioning backup SoftSIM 1/4");
      rc = nvs_write(&fs_backup, IMSI_BACKUP_ID, profile->IMSI, IMSI_LEN);
      if(rc < 0) {
        goto out_err;
      }

      LOG_INF("Provisioning backup SoftSIM 2/4");
      rc = nvs_write(&fs_backup, ICCID_BACKUP_ID, profile->ICCID, ICCID_LEN);
      if(rc < 0) {
        goto out_err;
      }

      LOG_INF("Provisioning backup SoftSIM 3/4");
      rc = nvs_write(&fs_backup, A001_BACKUP_ID, profile->A001, sizeof(profile->A001));
      if(rc < 0) {
        goto out_err;
      }

      LOG_INF("Provisioning backup SoftSIM 4/4");
      rc = nvs_write(&fs_backup, A004_BACKUP_ID, profile->A004, sizeof(profile->A004));
      if(rc < 0) {
        goto out_err;
      }
  }
#endif
#endif

  // IMSI 6f07
  struct cache_entry *entry = (struct cache_entry *)f_cache_find_by_name(IMSI_PATH, &fs_cache);

  LOG_INF("Provisioning SoftSIM 1/4");
  if (nvs_write(&fs, entry->key, profile->IMSI, IMSI_LEN) < 0) goto out_err;
  entry->_flags = 0;

  LOG_INF("Provisioning SoftSIM 2/4");
  entry = (struct cache_entry *)f_cache_find_by_name(ICCID_PATH, &fs_cache);
  if (nvs_write(&fs, entry->key, profile->ICCID, ICCID_LEN) < 0) {
    goto out_err;
  }
  entry->_flags = 0;

  LOG_INF("Provisioning SoftSIM 3/4");
  entry = (struct cache_entry *)f_cache_find_by_name(A001_PATH, &fs_cache);
  if (nvs_write(&fs, entry->key, profile->A001, sizeof(profile->A001)) < 0) goto out_err;
  entry->_flags = 0;

  LOG_INF("Provisioning SoftSIM 4/4");
  entry = (struct cache_entry *)f_cache_find_by_name(A004_PATH, &fs_cache);
  if (nvs_write(&fs, entry->key, profile->A004, sizeof(profile->A004)) < 0) goto out_err;
  entry->_flags = 0;

  // This is for test now. Removes uicc suspend flag as it isn't supported *yet* in softsim
  // Modem seems to 'deactivate' sim more often instead which isn't good :/

  // Enable the following to remove the uicc suspend flag

  // entry = (struct cache_entry *)f_cache_find_by_name("/3f00/2f08", &fs_cache);
  // if (nvs_write(&fs, entry->key, "0a05000000", 10) < 0) goto out_err;
  // entry->_flags = 0;

  LOG_INF("SoftSIM provisioned");
  return 0;

out_err:
  LOG_ERR("SoftSIM provisioning failed");
  return -1;
}

size_t impl_port_fwrite(const void *ptr, size_t size, size_t count, impl_port_FILE f) {
  struct cache_entry *entry = (struct cache_entry *)f;
  return f_cache_fwrite(&fs_cache, entry, ptr, size, count);
}

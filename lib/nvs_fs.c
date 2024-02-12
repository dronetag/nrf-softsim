#include <onomondo/softsim/fs_port.h>
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

LOG_MODULE_DECLARE(softsim_nvs, CONFIG_SOFTSIM_LOG_LEVEL);

static struct nvs_fs fs;
static struct nvs_fs fs_backup;
static struct ss_list fs_cache;

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

//  that the buffer is set. Either by allocating new memory or by
// stealing from another entry.
void read_nvs_to_cache(struct cache_entry *entry);
int port_provision_inner(struct ss_profile *profile, bool restore);

uint8_t fs_is_initialized = 0;
uint8_t nvs_is_initialized = 0;

int load_dirs() {
    uint8_t *data = NULL;
    size_t len = 0;
    ss_list_init(&fs_cache);

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

    ss_list_init(&fs_cache);
    generate_dir_table_from_blob(&fs_cache, data, len);

    if (ss_list_empty(&fs_cache)) goto out;
out:
    SS_FREE(data);
    return ss_list_empty(&fs_cache);
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
    rc = init_nvs_from_id(&fs, NVS_BACKUP_PARTITION_ID, false);
    if(rc) {
        return rc;
    }
#endif
    nvs_is_initialized++;
    return 0;
}

int init_fs() {
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

int deinit_fs() {
  // TODO: check if DIR entry is still valid
  // if not recreate and write.
  // Will NVS only commit if there are changes? If so, we can just recreate
  // and let NVS do the compare.

  struct cache_entry *cursor, *pre_cursor;

  /* Free all memory allocated by cache and
   * commit changes to NVS
   */
  SS_LIST_FOR_EACH_SAVE(&fs_cache, cursor, pre_cursor, struct cache_entry, list) {
    if (cursor->_b_dirty) {
      LOG_INF("Softsim stop - committing %s to NVS", cursor->name);
      nvs_write(&fs, cursor->key, cursor->buf, cursor->_l);
    }

    ss_list_remove(&cursor->list);

    if (cursor->buf) {
      SS_FREE(cursor->buf);
    }
    SS_FREE(cursor->name);
    SS_FREE(cursor);
  }

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
port_FILE port_fopen(char *path, char *mode) {
  struct cache_entry *cursor = NULL;
  int rc = 0;

  cursor = f_cache_find_by_name(path, &fs_cache);  //
  if (!cursor) {
    return NULL;
  }

  /**
   * Currently not used.
   * Could potentially be used in the future to re-arrange order.
   * Initial order is ordered by frequency already so not big optimizations can
   * be achieved.
   */
  if (cursor->_cache_hits < 0xFF) cursor->_cache_hits++;

  /**
   * File opened first time.
   */
  if (!cursor->_l) {
    rc = nvs_read(&fs, cursor->key, NULL, 0);

    if (rc < 0) {  // TODO: this can not happen.
      return NULL;
    } else {
      cursor->_l = rc;
    }
  }

  // Reset internal read/write pointer
  cursor->_p = 0;

  // Guarentee buffer contains valid data
  read_nvs_to_cache(cursor);
  return (void *)cursor;
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
size_t port_fread(void *ptr, size_t size, size_t nmemb, port_FILE fp) {
  if (nmemb == 0 || size == 0) {
    return 0;
  }
  struct cache_entry *entry = (struct cache_entry *)fp;

  size_t max_element_to_return = (entry->_l - entry->_p) / size;
  size_t element_to_return = nmemb > max_element_to_return ? max_element_to_return : nmemb;

  memcpy(ptr, entry->buf + entry->_p, element_to_return * size);

  entry->_p += element_to_return * size;  // move internal read/write pointer
  return element_to_return;
}

/**
 * @brief Makes sure internal buffer points to the actual data by fetching it
 * from NVS or protected storage if needed.
 * @param entry Pointer to a cache entry.
 */
void read_nvs_to_cache(struct cache_entry *entry) {
  struct cache_entry *tmp;

  /**
   * If entry has a buffer assigned it contains valid data.
   * Return early.
   */
  if (entry->buf) return;

  // best bet for a buffer we can reuse
  tmp = f_cache_find_buffer(entry, &fs_cache);

  uint8_t *buffer_to_use = NULL;
  size_t buffer_size = 0;

  if (tmp) {
    if (tmp->_b_dirty) {
      LOG_DBG("Cache entry %s is dirty, writing to NVS\n", tmp->name);
      nvs_write(&fs, tmp->key, tmp->buf, tmp->_l);
    }

    // should buffer be freed
    if (entry->_l > tmp->_b_size) {
      SS_FREE(tmp->buf);
    } else {  // or reused
      buffer_size = tmp->_b_size;
      buffer_to_use = tmp->buf;
      memset(buffer_to_use, 0, buffer_size);
    }

    tmp->buf = NULL;
    tmp->_b_size = 0;
    tmp->_b_dirty = 0;
  }

  if (!buffer_to_use) {
    buffer_size = entry->_l;
    LOG_DBG("Allocating buffer of size %d", buffer_size);
    buffer_to_use = SS_ALLOC_N(buffer_size * sizeof(uint8_t));
  }

  if (!buffer_to_use) {
    LOG_ERR("Failed to allocate buffer of size %d", buffer_size);
    return;
  }

  int rc = 0;

  rc = nvs_read(&fs, entry->key, buffer_to_use, entry->_l);

  if (rc < 0) {
    LOG_ERR("NVS read failed: %d\n", rc);
    SS_FREE(buffer_to_use);
    return;
  }

  entry->buf = buffer_to_use;
  entry->_b_size = buffer_size;
  entry->_b_dirty = 0;
}

int port_fclose(port_FILE fp) {
  struct cache_entry *entry = (struct cache_entry *)fp;

  if (!entry) {
    return -1;
  }

  if (entry->_flags & FS_READ_ONLY) {
    goto out;
  }

  if (entry->_flags & FS_COMMIT_ON_CLOSE) {
    if (entry->_b_dirty) {
      nvs_write(&fs, entry->key, entry->buf, entry->_l);
    }
    entry->_b_dirty = 0;
  }

out:
  entry->_p = 0;  // TODO: not needed?
  return 0;
}

int port_fseek(port_FILE fp, long offset, int whence) {
  struct cache_entry *entry = (struct cache_entry *)fp;

  if (!entry) {
    return -1;
  }

  if (whence == SEEK_SET) {
    entry->_p = offset;
  } else if (whence == SEEK_CUR) {
    entry->_p += offset;
    if (entry->_p >= entry->_l) entry->_p = entry->_l - 1;  // how is std c behaving here?
  } else if (whence == SEEK_END) {
    entry->_p = entry->_l - offset;
  }
  return 0;
}
long port_ftell(port_FILE fp) {
  struct cache_entry *entry = (struct cache_entry *)fp;

  if (!entry) {
    return -1;
  }

  return entry->_p;
}
int port_fputc(int c, port_FILE fp) {
  struct cache_entry *entry = (struct cache_entry *)fp;

  if (!entry) {
    return -1;
  }

  // writing beyond the end of the buffer
  if (entry->_p >= entry->_b_size) {
    uint8_t *old_buffer = entry->buf;
    size_t old_size = entry->_b_size;
    entry->buf = SS_ALLOC_N(entry->_b_size + 20);

    if (!entry->buf) {
      entry->buf = old_buffer;
      return -1;
    }

    memcpy(entry->buf, old_buffer, old_size);
    entry->_b_size += 20;
  }

  entry->buf[entry->_p++] = (uint8_t)c;
  entry->_b_dirty = 1;
  entry->_l = entry->_l >= entry->_p ? entry->_l : entry->_p;

  return c;
}
int port_access(const char *path, int amode) {
  return 0;
}  // TODO -> safe to omit for now. Internally SoftSIM will verify that a
   // directory exists after creation. Easier to guarentee since it isn't a
   // 'thing'
int port_mkdir(const char *, int) {
  return 0;
}  // don't care. We don't really obey directories (creating file
   // 'test/a/b/c.def) implicitly creates the directories

int port_remove(const char *path) {
  struct cache_entry *entry = f_cache_find_by_name(path, &fs_cache);

  if (!entry) {
    return -1;
  }

  ss_list_remove(&entry->list);  // doesn't free data

  // should we be smarter about this?
  nvs_delete(&fs, entry->key);

  if (entry->buf) SS_FREE(entry->buf);

  SS_FREE(entry->name);
  SS_FREE(entry);

  return 0;
}

// list for each { is_name_partial_match? {remove port_remove(cursor->name)} }
int port_rmdir(const char *) { return 0; }  // todo. Remove all entries with directory match.

static uint8_t default_imsi[] = {0x08, 0x09, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10};

int port_check_provisioned() {
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
int port_provision(struct ss_profile *profile) {
  return port_provision_inner(profile, false);
}

int port_provision_inner(struct ss_profile *profile, bool restore)
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

size_t port_fwrite(const void *prt, size_t size, size_t count, port_FILE f) {
  struct cache_entry *entry = (struct cache_entry *)f;

  if (!entry) {
    return -1;
  }

  const size_t requiredBufferSize = entry->_p + size * count;

  /**
   * In reality this rarely occurs. Potentially when upating SIM OTA
   */
  if (requiredBufferSize > entry->_b_size) {
    uint8_t *oldBuffer = entry->buf;
    const size_t oldSize = entry->_b_size;

    entry->buf = SS_ALLOC_N(requiredBufferSize);

    if (!entry->buf) {
      entry->buf = oldBuffer;
      return -1;
    } else {
      entry->_b_size = requiredBufferSize;
    }

    memcpy(entry->buf, oldBuffer, oldSize);
    SS_FREE(oldBuffer);
  }
  const size_t buffer_left = entry->_b_size - entry->_p;
  const size_t elements_to_copy = buffer_left > size * count ? count : buffer_left / size;

  const uint8_t content_is_different = memcmp(entry->buf + entry->_p, prt, size * elements_to_copy);

  if (content_is_different) {
    memcpy(entry->buf + entry->_p, prt, size * elements_to_copy);
    entry->_b_dirty = 1;
  }
  entry->_p += size * elements_to_copy;

  return elements_to_copy;
}

#include "f_cache.h"

#include <string.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <onomondo/softsim/list.h>
#include <onomondo/softsim/mem.h>


LOG_MODULE_REGISTER(softsim_fcache, CONFIG_SOFTSIM_LOG_LEVEL);

// #define PROFILE_DATA

void f_cache_init(struct cache_ctx *cache, bool static_cache, size_t max_entries, struct cache_strorage_funcs *funcs, bool free_entries)
{
    assert(funcs->length);
    assert(funcs->read);
    assert(funcs->remove);
    assert(funcs->write);
    ss_list_init(&cache->file_list);
    cache->storage_f = funcs;
    cache->free_entries = free_entries;
    cache->max_entries = max_entries;
    cache->static_cache = static_cache;
}

bool f_cache_empty(struct cache_ctx *cache)
{
    return ss_list_empty(&cache->file_list);
}

/**
 * @brief Makes sure internal buffer points to the actual data by fetching it
 * from NVS or protected storage if needed.
 * @param entry Pointer to a cache entry.
 */
int f_cache_read_to_cache(struct cache_ctx *cache, struct cache_entry *entry) {
    struct cache_entry *tmp;

    /**
   * If entry has a buffer assigned it contains valid data.
   * Return early.
   */
    if (!entry) return -EINVAL;
    /* Entry already loaded */
    if(entry->buf) {
        return 0;
    }

    // best bet for a buffer we can reuse
    tmp = f_cache_find_buffer(entry, cache);

    uint8_t *buffer_to_use = NULL;
    size_t buffer_size = 0;

    if (tmp) {
        if(cache->static_cache) {
            /* Cache should not try to free any entry. When files are pre-cached only frequently used files stays in the cache */
            return -ENOMEM;
        }
        if (tmp->_b_dirty) {
            LOG_DBG("Cache entry %s is dirty, writing to NVS\n", tmp->name);
            cache->storage_f->write(cache, tmp, tmp->buf, tmp->_l);
        }

        // should buffer be freed
        if (entry->_l > tmp->_b_size) {
            f_cache_free(cache, tmp->buf);
        } else {  // or reused
            buffer_size = tmp->_b_size;
            buffer_to_use = tmp->buf;
            memset(buffer_to_use, 0, buffer_size);
        }

        tmp->buf = NULL;
        tmp->_b_size = 0;
        tmp->_b_dirty = 0;

        if(cache->free_entries) {
            f_cache_delete_entry(cache, tmp);
        }
    }

    if (!buffer_to_use) {
        buffer_size = entry->_l;
        LOG_DBG("Allocating buffer of size %d", buffer_size);
        buffer_to_use = f_cache_alloc(cache, buffer_size * sizeof(uint8_t));
    }

    if (!buffer_to_use) {
        LOG_ERR("Failed to allocate buffer of size %d", buffer_size);
        return -ENOMEM;
    }

    int rc = 0;
    rc = cache->storage_f->read(cache, entry, buffer_to_use, entry->_l);

    if (rc < 0) {
        LOG_ERR("NVS read failed: %d\n", rc);
        f_cache_free(cache, buffer_to_use);
        return -ENOMEM;
    }

    entry->buf = buffer_to_use;
    entry->_b_size = buffer_size;
    entry->_b_dirty = 0;
    return 0;
}

// find a suitable cache entry with a buffer that can be re-used
struct cache_entry *f_cache_find_buffer(struct cache_entry *entry,
                                        struct cache_ctx *cache_ctx) {
  struct ss_list *cache = &cache_ctx->file_list;
  struct cache_entry *cursor;
  size_t min_hits_1 = 100, min_hits_2 = 100, min_hits_3 = 100;
  struct cache_entry *no_hits_no_write_existing_buff =
      NULL; // best case, no write needed, existing buffer with size >=
            // min_buf_size
  struct cache_entry *no_hits_no_write =
      NULL; // no write needed, existing buffer with size < min_buf_size
  struct cache_entry *no_hits = NULL; // write needed but low hit count

  size_t min_buf_size = entry->_l;
  size_t cached_entries = 0;

  SS_LIST_FOR_EACH(cache, cursor, struct cache_entry, list) {
    if (cursor->buf) {
      if (!cursor->_b_dirty && cursor->_b_size >= min_buf_size &&
          cursor->_cache_hits < min_hits_1) {
        min_hits_1 = cursor->_cache_hits;
        no_hits_no_write_existing_buff = cursor;
      }
      if (!cursor->_b_dirty && cursor->_cache_hits < min_hits_2) {
        min_hits_2 = cursor->_cache_hits;
        no_hits_no_write = cursor;
      }
      if (cursor->_cache_hits < min_hits_3) {
        min_hits_3 = cursor->_cache_hits;
        no_hits = cursor;
      }
      cached_entries++;
    }
  }

  // let cache grow to MAX_ENTRIES
  if (cached_entries < cache_ctx->max_entries)
    return NULL;

  if (no_hits_no_write_existing_buff)
    return no_hits_no_write_existing_buff;

  if (no_hits_no_write)
    return no_hits_no_write;

  if (no_hits)
    return no_hits;

  return NULL;
}

// lookup cache entry by name
struct cache_entry *f_cache_find_by_name(const char *name,
                                         struct cache_ctx *cache_ctx) {
  struct ss_list *cache = &cache_ctx->file_list;
  struct cache_entry *cursor;

  SS_LIST_FOR_EACH(cache, cursor, struct cache_entry, list) {
    if (strcmp(cursor->name, name) == 0) {
      return cursor;
    }
  }

  return NULL;
}

int f_cache_fread(struct cache_ctx *cache, struct cache_entry *entry, void *ptr, size_t size, size_t nmemb)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Entry read: %s", entry->name);
  size_t max_element_to_return = (entry->_l - entry->_p) / size;
  size_t element_to_return = nmemb > max_element_to_return ? max_element_to_return : nmemb;

  LOG_DBG("P: %d, size: %d", entry->_p, element_to_return * size);
  memcpy(ptr, entry->buf + entry->_p, element_to_return * size);
#ifdef PROFILE_DATA
  LOG_HEXDUMP_DBG(ptr, element_to_return * size, "Read data");
#endif

  entry->_p += element_to_return * size;  // move internal read/write pointer
  return element_to_return;
}

int f_cache_fclose(struct cache_ctx *cache, struct cache_entry *entry)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Closing cache entry: %s", entry->name);

  if (entry->_flags & FS_READ_ONLY) {
    goto out;
  }

  if (entry->_flags & FS_COMMIT_ON_CLOSE) {
    if (entry->_b_dirty) {
      cache->storage_f->write(cache, entry, entry->buf, entry->_l);
    }
    entry->_b_dirty = 0;
  }

out:
  entry->_p = 0;  // TODO: not needed?
  return 0;
}

#ifndef SEEK_SET
#define SEEK_SET 0 /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1 /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END 2 /* set file offset to EOF plus offset */
#endif


int f_cache_fseek(struct cache_ctx *cache, struct cache_entry *entry, long offset, int whence)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Entry seek: %s off: %ld, whence: %d", entry->name, offset, whence);

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

int f_cache_ftell(struct cache_ctx *cache, struct cache_entry *entry)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Entry tell: %s", entry->name);

  return entry->_p;
}

int f_cache_putc(struct cache_ctx *cache, struct cache_entry *entry, int c)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Entry putc: %s", entry->name);

  // writing beyond the end of the buffer
  if (entry->_p >= entry->_b_size) {
    uint8_t *old_buffer = entry->buf;
    size_t old_size = entry->_b_size;
    entry->buf = f_cache_alloc(cache, entry->_b_size + 20);

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

int f_cache_remove(struct cache_ctx *cache, struct cache_entry *entry)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Entry remove: %s", entry->name);

  ss_list_remove(&entry->list);  // doesn't free data

  // should we be smarter about this?
  cache->storage_f->remove(cache, entry);

  if (entry->buf) f_cache_free(cache, entry->buf);

  f_cache_free(cache, entry->name);
  f_cache_free(cache, entry);

  return 0;
}

size_t f_cache_fwrite(struct cache_ctx *cache, struct cache_entry *entry, const void *prt, size_t size, size_t count)
{
  if (!entry) {
    return -EINVAL;
  }
  LOG_DBG("Entry fwrite: %s", entry->name);

  const size_t requiredBufferSize = entry->_p + size * count;

  /**
   * In reality this rarely occurs. Potentially when upating SIM OTA
   */
  if (requiredBufferSize > entry->_b_size) {
    uint8_t *oldBuffer = entry->buf;
    const size_t oldSize = entry->_b_size;

    entry->buf = f_cache_alloc(cache, requiredBufferSize);

    if (!entry->buf) {
      entry->buf = oldBuffer;
      return -1;
    } else {
      entry->_b_size = requiredBufferSize;
    }

    memcpy(entry->buf, oldBuffer, oldSize);
    f_cache_free(cache, oldBuffer);
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

int f_cache_fopen(struct cache_ctx *cache, struct cache_entry *entry)
{
  LOG_DBG("Opening cache entry: %s", entry->name);
  /**
   * Currently not used.
   * Could potentially be used in the future to re-arrange order.
   * Initial order is ordered by frequency already so not big optimizations can
   * be achieved.
   */
  if (entry->_cache_hits < 0xFF) entry->_cache_hits++;

  // Reset internal read/write pointer
  entry->_p = 0;


  /**
    * File opened first time.
    */
  if (!entry->_l) {
    if(cache->storage_f->readall) {
      /* Short cut for FS with large penality for opening files and reading sizes */
      return cache->storage_f->readall(cache, entry, &entry->buf, &entry->_l);
    }

    int rc = cache->storage_f->length(cache, entry);
    if (rc < 0) {
      return -EINVAL;
    } else {
      entry->_l = rc;
    }
  }

  // Guarentee buffer contains valid data
  return f_cache_read_to_cache(cache, entry);
}

void f_cache_close(struct cache_ctx *cache)
{
  struct cache_entry *cursor, *pre_cursor;
  LOG_DBG("Closing cache");

  /* Free all memory allocated by cache and
   * commit changes to backend storage.
   */
  SS_LIST_FOR_EACH_SAVE(&cache->file_list, cursor, pre_cursor, struct cache_entry, list) {
    if (cursor->_b_dirty) {
      LOG_DBG("Softsim stop - committing %s", cursor->name);
      cache->storage_f->write(cache, cursor, cursor->buf, cursor->_l);
    }

    ss_list_remove(&cursor->list);

    if (cursor->buf) {
      f_cache_free(cache, cursor->buf);
    }
    f_cache_free(cache, cursor->name);
    f_cache_free(cache, cursor);
  }
}

/* Expects name to be preallocated by the f_cache_alloc */
int f_cache_create_entry(struct cache_ctx *cache, uint16_t key, char *name, uint16_t flags, struct cache_entry **out_entry)
{
  struct ss_list *dirs = &cache->file_list;
  struct cache_entry *entry = f_cache_alloc(cache, sizeof(struct cache_entry));
  if(!entry) {
    return -ENOMEM;
  }
  memset(entry, 0, sizeof(struct cache_entry));

  entry->key = key;
  entry->name = name;
  entry->_flags =flags;
  entry->buf = NULL;
  ss_list_put(dirs, &entry->list);
  *out_entry = entry;
  return 0;
}

void f_cache_free(struct cache_ctx *cache, void *ptr)
{
  if(cache->storage_f->free) {
    cache->storage_f->free(ptr);
  } else {
    SS_FREE(ptr);
  }
}

void *f_cache_alloc(struct cache_ctx *cache, size_t len)
{
  if(cache->storage_f->alloc) {
    return cache->storage_f->alloc(len);
  } else {
    return SS_ALLOC_N(len);
  }
}

void f_cache_delete_entry(struct cache_ctx *cache, struct cache_entry *entry)
{
  f_cache_free(cache, entry->name);
  f_cache_free(cache, entry);
  ss_list_remove(&entry->list);
}

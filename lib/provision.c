
#include "provision.h"

#include <stdlib.h>
#include <string.h>
#include <zephyr/sys/printk.h>
#include <onomondo/softsim/mem.h>
#include "f_cache.h"
char storage_path[] = "";

/**
 * @brief TODO: move this function to a more appropriate place
 * It is used to generate the directory structure based on the content in the
 * "DIR" file. The DIR file encodes ID (used to locate actual file in flash) and
 * name of the file.
 *

 * @param dirs linked list to populate
 * @param blob pointer to blob of data
 * @param size size of blob
 */
void generate_dir_table_from_blob(struct cache_ctx *cache, uint8_t *blob, size_t size) {
  struct ss_list *dirs = &cache->file_list;
  size_t cursor = 0;
  while (cursor < size) {
    uint8_t len = blob[cursor++];
    uint16_t id = (blob[cursor] << 8) | blob[cursor + 1];

    cursor += 2;

    char *name = SS_ALLOC_N(len + 1);
    memcpy(name, &blob[cursor], len);
    name[len] = '\0';
    cursor += (len);

    struct cache_entry *entry;
    /* No Return ? Maybe fix check if allocation was susccessful */
    f_cache_create_entry(cache, id, name, (id & 0xFF00) >> 8, &entry);
  }
}

#ifndef _ZEPHYR_ONOMONDO_STATICFILES
#define _ZEPHYR_ONOMONDO_STATICFILES

#include <stdint.h>

typedef struct {
    const char *name; const uint8_t *data; const uint32_t size; const uint16_t nvs_key;
} sf_file_t;

typedef struct {
    const char *name;
} sf_dir_t;

extern const sf_file_t *onomondo_sf_files;
extern const sf_dir_t *onomondo_sf_dirs;
extern const uint32_t onomondo_sf_files_len;
extern const uint32_t onomondo_sf_dirs_len;
extern const uint32_t onomondo_sf_files_names_len;


#endif

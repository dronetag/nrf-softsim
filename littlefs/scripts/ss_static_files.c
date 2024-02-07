
#include <stdint.h>
typedef struct {const char *name; const uint8_t *data; const uint32_t size;} file_t;
typedef struct {const char *name;} dir_t;
const dir_t dirs[] = {
{.name = "/3f00"}, 
{.name = "/3f00/7ff0"}
};
const file_t files[] = {
{.name = "/3f00.def", .data = {0x62,0x10,0x82,0x02,0x78,0x21,0x83,0x02,0x3f,0x00,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x0f}, .size = 18}, 
{.name = "/3f00/2fe2", .data = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99}, .size = 10}, 
{.name = "/3f00/5f100001.def", .data = {0x62,0x10,0x82,0x05,0x02,0x21,0x00,0x02,0x1f,0x83,0x04,0x5f,0x10,0x00,0x01,0x80,0x01,0x3e}, .size = 18}, 
{.name = "/3f00/a103", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a104", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a10d", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a10c", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a105", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a102", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a10b", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a10e", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a11f", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a11a", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/2f05", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 10}, 
{.name = "/3f00/a111", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/7ff0.def", .data = {0x62,0x22,0x82,0x02,0x78,0x21,0x83,0x02,0x7f,0xf0,0x84,0x10,0xa0,0x00,0x00,0x00,0x87,0x10,0x02,0xff,0xff,0xff,0xff,0x89,0x07,0x09,0x00,0x00,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x0f}, .size = 36}, 
{.name = "/3f00/a116", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a120", .data = {0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a118", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/2fe2.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x2f,0xe2,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x03,0x80,0x02,0x00,0x0a,0x88,0x01,0x10}, .size = 25}, 
{.name = "/3f00/a119", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a117", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a110", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/2f08.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x2f,0x08,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x02,0x80,0x02,0x00,0x05,0x88,0x01,0x40}, .size = 25}, 
{.name = "/3f00/a001", .data = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x00}, .size = 33}, 
{.name = "/3f00/a001.def", .data = {0x62,0x16,0x82,0x02,0x41,0x21,0x83,0x02,0xa0,0x01,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x06,0x80,0x02,0x00,0x21,0x88,0x00}, .size = 24}, 
{.name = "/3f00/2f00.def", .data = {0x62,0x1a,0x82,0x05,0x42,0x21,0x00,0x26,0x02,0x83,0x02,0x2f,0x00,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x02,0x80,0x02,0x00,0x4c,0x88,0x01,0xf0}, .size = 28}, 
{.name = "/3f00/a003.def", .data = {0x62,0x19,0x82,0x05,0x42,0x21,0x00,0x16,0x03,0x83,0x02,0xa0,0x03,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x06,0x80,0x02,0x00,0x42,0x88,0x00}, .size = 27}, 
{.name = "/3f00/a002.def", .data = {0x62,0x16,0x82,0x02,0x41,0x21,0x83,0x02,0xa0,0x02,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x06,0x80,0x02,0x01,0x08,0x88,0x00}, .size = 24}, 
{.name = "/3f00/a1df1d01", .data = {0xa0,0x00,0x00,0x00,0x87,0x10,0x02,0xff,0xff,0xff,0xff,0x89,0x07,0x09,0x00,0x00,0x7f,0xf0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 288}, 
{.name = "/3f00/2f05.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x2f,0x05,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x01,0x80,0x02,0x00,0x0a,0x88,0x01,0x28}, .size = 25}, 
{.name = "/3f00/a109", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a107", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a100", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a10f", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/2f06.def", .data = {0x62,0x1a,0x82,0x05,0x42,0x21,0x00,0x28,0x10,0x83,0x02,0x2f,0x06,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x02,0x80,0x02,0x02,0x80,0x88,0x01,0x30}, .size = 28}, 
{.name = "/3f00/a10a", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a005.def", .data = {0x62,0x19,0x82,0x05,0x42,0x21,0x00,0x0b,0x03,0x83,0x02,0xa0,0x05,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x06,0x80,0x02,0x00,0x21,0x88,0x00}, .size = 27}, 
{.name = "/3f00/a004.def", .data = {0x62,0x19,0x82,0x05,0x42,0x21,0x00,0x26,0x03,0x83,0x02,0xa0,0x04,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x06,0x80,0x02,0x00,0x72,0x88,0x00}, .size = 27}, 
{.name = "/3f00/a101", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a106", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a108", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a115", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a1df1d01.def", .data = {0x62,0x11,0x82,0x05,0x02,0x21,0x00,0x12,0x10,0x83,0x04,0xa1,0xdf,0x1d,0x01,0x80,0x02,0x01,0x20}, .size = 19}, 
{.name = "/3f00/a112", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a11b", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a11e", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/2f08", .data = {0x3c,0x05,0x02,0x00,0x00}, .size = 5}, 
{.name = "/3f00/2f06", .data = {0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 640}, 
{.name = "/3f00/a100.def", .data = {0x62,0x16,0x82,0x02,0x41,0x21,0x83,0x02,0xa0,0x02,0x8a,0x01,0x05,0x8b,0x03,0x2f,0x06,0x06,0x80,0x02,0x01,0x08,0x88,0x00}, .size = 24}, 
{.name = "/3f00/5f100001", .data = {0xff,0xff,0x2f,0xe2,0xff,0xff,0xff,0xff,0x2f,0x05,0x2f,0x06,0xff,0xff,0x2f,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x2f,0x00,0xff,0xff}, .size = 62}, 
{.name = "/3f00/a113", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a114", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/2f00", .data = {0x61,0x19,0x4f,0x10,0xa0,0x00,0x00,0x00,0x87,0x10,0x02,0xff,0xff,0xff,0xff,0x89,0x07,0x09,0x00,0x00,0x50,0x05,0x55,0x53,0x69,0x6d,0x31,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 76}, 
{.name = "/3f00/a11d", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a11c", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, .size = 8}, 
{.name = "/3f00/a004", .data = {0xb0,0x00,0x11,0x06,0x03,0x03,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0x01,0x23,0x45,0x67,0x01,0x23,0x45,0x67,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 114}, 
{.name = "/3f00/a003", .data = {0x00,0x03,0x00,0x0a,0x00,0x01,0x31,0x32,0x33,0x34,0xff,0xff,0xff,0xff,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x00,0x03,0x00,0x0a,0x00,0x81,0x31,0x32,0x33,0x34,0xff,0xff,0xff,0xff,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x01,0x03,0x00,0x00,0x00,0x0a,0x31,0x32,0x33,0x34,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 66}, 
{.name = "/3f00/a002", .data = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00}, .size = 264}, 
{.name = "/3f00/a005", .data = {0xb0,0x00,0x11,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 33}, 
{.name = "/3f00/7ff0/6f31.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x31,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x05,0x80,0x02,0x00,0x01,0x88,0x01,0x90}, .size = 25}, 
{.name = "/3f00/7ff0/6fe4", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 54}, 
{.name = "/3f00/7ff0/6fe4.def", .data = {0x62,0x1a,0x82,0x05,0x42,0x21,0x00,0x36,0x01,0x83,0x02,0x6f,0xe4,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x36,0x88,0x01,0xc0}, .size = 28}, 
{.name = "/3f00/7ff0/6fe3", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x01}, .size = 18}, 
{.name = "/3f00/7ff0/5f100001.def", .data = {0x62,0x10,0x82,0x05,0x02,0x21,0x00,0x02,0x1f,0x83,0x04,0x5f,0x10,0x00,0x01,0x80,0x01,0x3e}, .size = 18}, 
{.name = "/3f00/7ff0/6f42", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xe5,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x91,0x44,0x77,0x79,0x07,0x84,0x84,0xff,0xff,0xff,0xff,0xff,0x00,0xa8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 104}, 
{.name = "/3f00/7ff0/6f73", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0xff,0x01}, .size = 14}, 
{.name = "/3f00/7ff0/6f08.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x08,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x21,0x88,0x01,0x40}, .size = 25}, 
{.name = "/3f00/7ff0/6fe3.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0xe3,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x12,0x88,0x01,0x18}, .size = 25}, 
{.name = "/3f00/7ff0/6f7e", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xff,0x01}, .size = 11}, 
{.name = "/3f00/7ff0/6f7b", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 12}, 
{.name = "/3f00/7ff0/6f09.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x09,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x21,0x88,0x01,0x48}, .size = 25}, 
{.name = "/3f00/7ff0/6fc4.def", .data = {0x62,0x16,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0xc4,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x40,0x88,0x00}, .size = 24}, 
{.name = "/3f00/7ff0/6f05", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 10}, 
{.name = "/3f00/7ff0/6f78.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x78,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x05,0x80,0x02,0x00,0x02,0x88,0x01,0x30}, .size = 25}, 
{.name = "/3f00/7ff0/6fad", .data = {0x01,0x00,0x08,0x03}, .size = 4}, 
{.name = "/3f00/7ff0/6f5b.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x5b,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x06,0x88,0x01,0x78}, .size = 25}, 
{.name = "/3f00/7ff0/6fb7.def", .data = {0x62,0x1a,0x82,0x05,0x42,0x21,0x00,0x10,0x05,0x83,0x02,0x6f,0xb7,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x02,0x80,0x02,0x00,0x50,0x88,0x01,0x08}, .size = 28}, 
{.name = "/3f00/7ff0/6f5c.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x5c,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x05,0x80,0x02,0x00,0x03,0x88,0x01,0x80}, .size = 25}, 
{.name = "/3f00/7ff0/6f42.def", .data = {0x62,0x19,0x82,0x05,0x42,0x21,0x00,0x34,0x02,0x83,0x02,0x6f,0x42,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x02,0x80,0x02,0x00,0x68,0x88,0x00}, .size = 27}, 
{.name = "/3f00/7ff0/6f73.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x73,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x0e,0x88,0x01,0x60}, .size = 25}, 
{.name = "/3f00/7ff0/6f7e.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x7e,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x0b,0x88,0x01,0x58}, .size = 25}, 
{.name = "/3f00/7ff0/6fc4", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 64}, 
{.name = "/3f00/7ff0/6fad.def", .data = {0x62,0x16,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0xad,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x02,0x80,0x02,0x00,0x04,0x88,0x00}, .size = 24}, 
{.name = "/3f00/7ff0/6f7b.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x7b,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x04,0x80,0x02,0x00,0x0c,0x88,0x01,0x68}, .size = 25}, 
{.name = "/3f00/7ff0/6f78", .data = {0x03,0xff}, .size = 2}, 
{.name = "/3f00/7ff0/6f38.def", .data = {0x62,0x16,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x38,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x05,0x80,0x02,0x00,0x0f,0x88,0x00}, .size = 24}, 
{.name = "/3f00/7ff0/6f07", .data = {0x08,0x09,0x10,0x10,0x00,0x00,0x00,0x00,0x10}, .size = 9}, 
{.name = "/3f00/7ff0/6f38", .data = {0x00,0x08,0x00,0x0c,0x21,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00}, .size = 15}, 
{.name = "/3f00/7ff0/6f31", .data = {0x05}, .size = 1}, 
{.name = "/3f00/7ff0/6f05.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x05,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x01,0x80,0x02,0x00,0x0a,0x88,0x01,0x10}, .size = 25}, 
{.name = "/3f00/7ff0/6f5c", .data = {0xff,0xff,0xff}, .size = 3}, 
{.name = "/3f00/7ff0/6f09", .data = {0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 33}, 
{.name = "/3f00/7ff0/6f07.def", .data = {0x62,0x17,0x82,0x02,0x41,0x21,0x83,0x02,0x6f,0x07,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x05,0x80,0x02,0x00,0x09,0x88,0x01,0x38}, .size = 25}, 
{.name = "/3f00/7ff0/5f100001", .data = {0x6f,0xb7,0x6f,0x05,0x6f,0xe3,0xff,0xff,0xff,0xff,0x6f,0x78,0x6f,0x07,0x6f,0x08,0x6f,0x09,0xff,0xff,0x6f,0x7e,0x6f,0x73,0x6f,0x7b,0xff,0xff,0x6f,0x5b,0x6f,0x5c,0xff,0xff,0x6f,0x31,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x6f,0x06,0x6f,0xe4,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 62}, 
{.name = "/3f00/7ff0/6f5b", .data = {0xf0,0x00,0x00,0xf0,0x00,0x00}, .size = 6}, 
{.name = "/3f00/7ff0/6f08", .data = {0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 33}, 
{.name = "/3f00/7ff0/6f06", .data = {0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0x90,0x00,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0xa4,0x06,0x83,0x01,0x01,0x95,0x01,0x08,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x01,0x01,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x02,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0x80,0x01,0x00,0xa4,0x06,0x83,0x01,0x0a,0x95,0x01,0x08,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, .size = 640}, 
{.name = "/3f00/7ff0/6f06.def", .data = {0x62,0x1a,0x82,0x05,0x42,0x21,0x00,0x28,0x10,0x83,0x02,0x6f,0x06,0x8a,0x01,0x05,0x8b,0x03,0x6f,0x06,0x02,0x80,0x02,0x02,0x80,0x88,0x01,0xb8}, .size = 28}, 
{.name = "/3f00/7ff0/6fb7", .data = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00}, .size = 80}
};
const uint32_t num_files = 100;
const uint32_t num_dirs = 3;

#include <row.h>
#include <string.h>

void SerialiseRow(Row* source, void* destination, const Field fields[])
{
    char* dest = (char*)destination;
    const char* src = (char*)source;

    for (size_t i = 0; fields[i].size != 0; i++) {
        memcpy(dest, src + fields[i].offset, fields[i].size);
        dest += fields[i].size;
    }
}

void DeserialiseRow(void* source, Row* destination, const Field fields[])
{
    const char* src = (const char*)source;
    char* dest = (char*)destination;

    for (size_t i = 0; fields[i].size != 0; i++) {
        if (fields[i].size == sizeof(uint32_t)) {
            // For integer fields, copy directly
            memcpy(dest + fields[i].offset, src, fields[i].size);
        } else {
            // For string fields, use strncpy to ensure null-termination
            strncpy(dest + fields[i].offset, src, fields[i].size);
            ((char*)(dest + fields[i].offset))[fields[i].size - 1] = '\0'; // Ensure null-termination
        }
        src += fields[i].size;
    }
}

#include <row.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void SerialiseRow(Row* source, void* destination, const Field fields[])
{
    char* dest = (char*)destination;
    const char* src = (char*)source;

    for (uint32_t i = 0; fields[i].size != 0; i++) {
        memcpy(dest, src + fields[i].offset, fields[i].size);

        dest += fields[i].size;
    }
}

void DeserialiseRow(void* source, Row* destination, const Field fields[])
{

    const char* src = (const char*)source;
    char* dest = (char*)destination;

    for (uint32_t i = 0; fields[i].size != 0; i++) {

        memcpy(dest + fields[i].offset, src, fields[i].size);

        src += fields[i].size;
    }
}

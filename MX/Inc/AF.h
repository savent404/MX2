/*
MIT License

Copyright (c) 2016 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _AF_H_
#define _AF_H_

/* This port will deal with ".wav" format */

#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)
struct _AF_PCM {
    /* Chunk block
       should be "RIFF" */
    char ChunkID[4];

    /* Chunk Size
       Size without ChunkID and ChunkSize */
    uint32_t ChunkSize;

    char    Format[4];

    char    Subchunk1ID[4];

    uint32_t Subchunk1Size;

    /* Format, if it's PCM Format,
       should be 0x0001 */
    uint16_t AudioFormat;

    uint16_t NumChannels;

    uint32_t SampleRate;

    uint32_t ByteRate;

    uint16_t BlockAlign;

    uint16_t BitsPerSample;
};

struct _AF_DATA {
    /* suoport to be 'data' */
    char ChunkID[4];

    uint32_t size;
};

#ifdef __cplusplus
}
#endif

#endif

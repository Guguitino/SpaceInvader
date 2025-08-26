#if !defined (WIN32_SPACE_INVADER_H)

#include "SpaceInvader.h"

typedef struct win32_screen_buffer win32_screen_buffer;
struct win32_screen_buffer
{
    CONSOLE_SCREEN_BUFFER_INFO Info;
    void *Memory;
    COORD ScreenSize;
    COORD GameSize;
    int Pitch;
    int BytesPerPixel;
};

typedef struct win32_state win32_state;
struct win32_state
{
    HANDLE StdInputHandle;
    HANDLE StdOutputHandle;

    uint64 TotalSize;
    void *GameMemoryBlock;
};

#define WIN32_SPACE_INVADER_H
#endif
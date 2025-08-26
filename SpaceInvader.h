#if !defined (SPACE_INVADER_H)

#include <stdint.h>
#include <Windows.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

#define Assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes((uint64)Value) * 1024)
#define Terabytes(Value) (Gigabytes((uint64)Value) * 1024)

#define BACKGROUND_WHITE 0x00f0
#define SPACE_CHAR 

typedef struct char_info char_info;
struct char_info {
    unsigned short UnicodeChar;
    unsigned short Attributes;
};

typedef struct game_offscreen_buffer game_offscreen_buffer;
struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

typedef struct game_memory game_memory;
struct game_memory
{
    bool32 IsInitialized;
    uint64 PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero by the platform

    uint64 TransientStorageSize;
    void *TransientStorage; // REQUIRED to be cleared to zero by the platform
};

typedef struct game_key_state game_key_state;
struct game_key_state
{
    bool32 IsPressed;
    bool32 IsDown;
    bool32 IsReleased;
};

typedef struct game_state game_state;
struct game_state
{
    int PlayerX;
    int PlayerY;
};

typedef struct game_input game_input ;
struct game_input
{
    union{
        struct game_key_state Buttons[6];
        struct
        {
            game_key_state Up;
            game_key_state Down;
            game_key_state Left;
            game_key_state Right;
    
            game_key_state Start;
            game_key_state Quit;
        };
    };
};

void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *ScreenBuffer);

#define SPACE_INVADER_H
#endif
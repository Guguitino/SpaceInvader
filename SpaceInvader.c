#include "SpaceInvader.h"

internal void RenderWeirdGradient(game_offscreen_buffer *Buffer)
{
    void *Row = (void *)Buffer->Memory;

    for (int Y = 0; Y < Buffer->Height; ++Y)
    {
        char_info *PixelChar = (char_info *)Row;
        for (int X = 0; X < Buffer->Width; ++X)
        {
            if(X == 0)
            {
                PixelChar->UnicodeChar = (unsigned short)(48+(Y%10));
                PixelChar->Attributes = BACKGROUND_RED; // coucou :)
            }
            else if((X+Y) % 2 == 0)
            {
                
                PixelChar->UnicodeChar = (unsigned short)(48+(X%10));
                PixelChar->Attributes = BACKGROUND_WHITE;
            }
            else {
                
                PixelChar->UnicodeChar = (unsigned short)(48+(X%10));
                PixelChar->Attributes = BACKGROUND_BLUE;
            }

            PixelChar ++; 
        }
        Row += Buffer->Pitch;
    }
}

void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *ScreenBuffer)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;

    if (!Memory->IsInitialized)
    {
        //Memory init
        Memory->IsInitialized = true;
    }
    RenderWeirdGradient(ScreenBuffer);
}
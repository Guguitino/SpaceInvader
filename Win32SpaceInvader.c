#include "Win32SpaceInvader.h"
#include "stdio.h"
#include <consoleapi2.h>
#include <consoleapi3.h>
#include <wincontypes.h>
#include <windows.h>

global_variable bool GlobalRunning;
global_variable int64 GlobalPerfCountFrequency;

internal void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL) == 0) {
        MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
        ExitProcess(dw);
    }

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    ExitProcess(dw);
}

internal LARGE_INTEGER Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

internal double Win32GetSecondElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    LARGE_INTEGER EndCounter;
    QueryPerformanceCounter(&EndCounter);
    double Result = ((double)End.QuadPart - (double)Start.QuadPart) / (double)GlobalPerfCountFrequency;
    return Result;
}

internal void KeyProcess(game_key_state *key, KEY_EVENT_RECORD KeyEvent)
{
    if (KeyEvent.bKeyDown)
    {
        key->IsDown = true;
        key->IsPressed = true;
    }
    else
    {
        key->IsDown = false;
        key->IsReleased = true;
    }
}

internal void KeyboardEventProcess(game_input *Input, KEY_EVENT_RECORD KeyEventRecord)
{
    uint32 VKCode = KeyEventRecord.wVirtualKeyCode;

    if (VKCode == 'Z')
    {
        KeyProcess(&Input->Up, KeyEventRecord);
    }
    else if (VKCode == 'Q')
    {
        KeyProcess(&Input->Left, KeyEventRecord);
    }
    else if (VKCode == 'S')
    {
        KeyProcess(&Input->Down, KeyEventRecord);
    }
    else if (VKCode == 'D')
    {
        KeyProcess(&Input->Right, KeyEventRecord);
    }
    else if (VKCode == VK_UP)
    {
        KeyProcess(&Input->Up, KeyEventRecord);
    }
    else if (VKCode == VK_LEFT)
    {
        KeyProcess(&Input->Left, KeyEventRecord);
    }
    else if (VKCode == VK_DOWN)
    {
        KeyProcess(&Input->Down, KeyEventRecord);
    }
    else if (VKCode == VK_RIGHT)
    {
        KeyProcess(&Input->Right, KeyEventRecord);
    }
    else if (VKCode == VK_SPACE)
    {
        KeyProcess(&Input->Start, KeyEventRecord);
    }
    else if (VKCode == VK_ESCAPE)
    {
        KeyProcess(&Input->Quit, KeyEventRecord);
        GlobalRunning = false;
    }
}

internal void ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeRecord)
{
    printf("Resize event\n");
    printf("Console screen buffer is %d columns by %d rows.\n", WindowBufferSizeRecord.dwSize.X, WindowBufferSizeRecord.dwSize.Y);
}

internal void ProcessingConsoleInput(HANDLE StdInputHandle, game_input *Input)
{
    
    for (int ButtonIndex = 0; ButtonIndex < ArrayCount(Input->Buttons); ButtonIndex++)
    {
        Input->Buttons[ButtonIndex].IsPressed = 0;
        Input->Buttons[ButtonIndex].IsReleased = 0;
    }

    DWORD NumberOfInputEvents;
    GetNumberOfConsoleInputEvents(StdInputHandle, &NumberOfInputEvents);
    if (NumberOfInputEvents > 0)
    {
        INPUT_RECORD InputBuffer[128];
        DWORD NumRead;
        if (!ReadConsoleInput(StdInputHandle, InputBuffer, 128, &NumRead))
        {
            ErrorExit("ReadConsoleInput");
        }
        for (int i = 0; i < NumRead; i++)
        {
            switch (InputBuffer[i].EventType)
            {
            case KEY_EVENT: // keyboard input
                KeyboardEventProcess(Input, InputBuffer[i].Event.KeyEvent);
                break;

            case WINDOW_BUFFER_SIZE_EVENT: // disregard screen buffer resizing
            case MOUSE_EVENT:              // disregard mouse input
            case FOCUS_EVENT:              // disregard focus events
            case MENU_EVENT:               // disregard menu events
                break;

            default:
                ErrorExit("Unknown event type");
                break;
            }
        }
    }
}

internal void ClearScreenBuffer(win32_screen_buffer *Win32ScreenBuffer)
{
    CHAR_INFO *Buffer = (CHAR_INFO *)Win32ScreenBuffer->Memory;
    int X = Win32ScreenBuffer->GameSize.X;
    int Y = Win32ScreenBuffer->GameSize.Y;
    for (int i=0; i < X * Y; ++i) {

        // Fill it with white-backgrounded spaces
        Buffer[i].Char.AsciiChar = ' ';
        Buffer[i].Attributes = 
            BACKGROUND_BLUE |
            BACKGROUND_GREEN |
            BACKGROUND_RED |
            BACKGROUND_INTENSITY;
    }
}

int main(int argc, char *argv[])
{
    float GameUpdateFrequency = 20.0f;
    double TargetSecondsPerFrame = 1.0 / (double)GameUpdateFrequency;

    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    LARGE_INTEGER LastCounter = Win32GetWallClock();

    win32_state Win32State = {};
    Win32State.StdInputHandle = GetStdHandle(STD_INPUT_HANDLE);
    Win32State.StdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (Win32State.StdInputHandle == INVALID_HANDLE_VALUE || Win32State.StdOutputHandle == INVALID_HANDLE_VALUE)
    {
        ErrorExit("GetStdHandle");
    }

    DWORD ConsoleSavedOldMode;
    if (!GetConsoleMode(Win32State.StdInputHandle, &ConsoleSavedOldMode))
        ErrorExit("GetConsoleMode");
    //(ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE
    if (!SetConsoleMode(Win32State.StdInputHandle, 0))
    {
        ErrorExit("GetConsoleMode");
    }

    win32_screen_buffer Win32ScreenBuffer = {};
    if (!GetConsoleScreenBufferInfo(Win32State.StdOutputHandle, &Win32ScreenBuffer.Info))
    {
        ErrorExit("GetConsoleScreenBufferInfo");
    }

    CONSOLE_FONT_INFOEX FontInfo;
    FontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    BOOL GetCurrentFontRes = GetCurrentConsoleFontEx(Win32State.StdOutputHandle, FALSE, &FontInfo);
    if (!GetCurrentFontRes)
    {
        ErrorExit("GetCurrentConsoleFont");
    }
    COORD FontSize = GetConsoleFontSize(Win32State.StdOutputHandle, FontInfo.nFont);
    if (!FontSize.X && !FontSize.Y)
    {
        ErrorExit("GetCurrentConsoleFont");
    }
    FontInfo.dwFontSize.X = FontSize.Y;
    FontInfo.dwFontSize.Y = FontSize.Y;
    BOOL SetFontRes = SetCurrentConsoleFontEx(Win32State.StdOutputHandle, FALSE, &FontInfo);
    if (!SetFontRes)
    {
        ErrorExit("SetCurrentConsoleFont");
    }


    Win32ScreenBuffer.ScreenSize = Win32ScreenBuffer.Info.dwSize;
    Win32ScreenBuffer.GameSize.X = Win32ScreenBuffer.ScreenSize.X; // 1005
    Win32ScreenBuffer.GameSize.Y = Win32ScreenBuffer.ScreenSize.Y; // 628
    Win32ScreenBuffer.BytesPerPixel = sizeof(CHAR_INFO);
    Win32ScreenBuffer.Pitch = Win32ScreenBuffer.ScreenSize.X * Win32ScreenBuffer.BytesPerPixel;
    SIZE_T ScreenBufferMemorySize = Win32ScreenBuffer.BytesPerPixel * Win32ScreenBuffer.GameSize.X * Win32ScreenBuffer.GameSize.Y;
    Win32ScreenBuffer.Memory = VirtualAlloc(0, ScreenBufferMemorySize, MEM_COMMIT, PAGE_READWRITE);

    ClearScreenBuffer(&Win32ScreenBuffer);

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(64);
    GameMemory.TransientStorageSize = Megabytes(256);

#if SPACEINVADER_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes((uint64)2);
#else
    LPVOID BaseAddress = 0;
#endif

    Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
    GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

    game_input Input = {};

    GlobalRunning = true;
    while (GlobalRunning)
    {
        // Input Processing
        ProcessingConsoleInput(Win32State.StdInputHandle, &Input);

        // Button Testing
        for (int ButtonIndex = 0; ButtonIndex < ArrayCount(Input.Buttons); ButtonIndex++)
        {
            game_key_state key = Input.Buttons[ButtonIndex];
            if (ButtonIndex == 0)
            {
                // printf("Up key state : IsDown = %d, WasDown = %d\n", key.IsDown, key.WasDown);
            }

            if (key.IsPressed)
            {
                printf("Key pressed\n");
            }
            else if (key.IsDown && !key.IsPressed)
            {
                printf("Key held\n");
            }
            else if (key.IsReleased)
            {
                printf("Key released\n");
            }
        }

        game_offscreen_buffer ScreenBuffer = {};
        ScreenBuffer.Memory = Win32ScreenBuffer.Memory;
        ScreenBuffer.Width = Win32ScreenBuffer.GameSize.X;
        ScreenBuffer.Height = Win32ScreenBuffer.GameSize.Y;
        ScreenBuffer.Pitch = Win32ScreenBuffer.Pitch;
        ScreenBuffer.BytesPerPixel = Win32ScreenBuffer.BytesPerPixel;

        ClearScreenBuffer(&Win32ScreenBuffer);

        GameUpdateAndRender(&GameMemory, &Input, &ScreenBuffer);

        COORD BufferCoords = {0,0};
        SMALL_RECT WriteRegion = {BufferCoords.X, BufferCoords.Y, BufferCoords.X + Win32ScreenBuffer.ScreenSize.X, BufferCoords.Y + Win32ScreenBuffer.ScreenSize.Y};
        bool32 res = WriteConsoleOutputA(Win32State.StdOutputHandle, (CHAR_INFO *)Win32ScreenBuffer.Memory, Win32ScreenBuffer.ScreenSize, BufferCoords, &WriteRegion);
        if(!res)
        {
            ErrorExit("WriteConsoleOutput");
        }


        // Time Managing
        LARGE_INTEGER WorkCounter = Win32GetWallClock();
        double WorkSecondsElapsed = Win32GetSecondElapsed(LastCounter, WorkCounter);

        double SecondsElapsedForFrame = WorkSecondsElapsed;
        if (SecondsElapsedForFrame < TargetSecondsPerFrame)
        {
            if (SleepIsGranular)
            {
                DWORD SleepMS = (DWORD)(1000.0 * (TargetSecondsPerFrame - SecondsElapsedForFrame));

                if (SleepMS > 0)
                {
                    Sleep(SleepMS);
                }
            }

            double TestSecondsElapsedForFrame = Win32GetSecondElapsed(LastCounter, Win32GetWallClock());
            if (TestSecondsElapsedForFrame <= TargetSecondsPerFrame - 0.001f)
            {
                // Log Miss here

                printf("Sleeped for to long ??\n");
            }

            while (SecondsElapsedForFrame < TargetSecondsPerFrame)
            {
                LARGE_INTEGER CheckCounter = Win32GetWallClock();
                SecondsElapsedForFrame = Win32GetSecondElapsed(LastCounter, CheckCounter);
            }
        }
        else
        {
            // TODO Missed a frame !!
            // TODO Logging
            printf("Missed a frame !!\n");
        }

        LARGE_INTEGER EndCounter = Win32GetWallClock();
        LastCounter = EndCounter;
    }
}
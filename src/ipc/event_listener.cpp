#include "event_listener.h"
#include <Windows.h>
#include "defines/defines.h"
#include "spdlog/spdlog.h"
#include "utils/common_utils.h"
#include "ipc.h"
#include "defines/globals.h"

void EventListenerLoopThread()
{
    int numEvents = FANY_IME_EVENT_ARRAY.size();
    if (numEvents == 0)
    {
        spdlog::warn("No events to wait for in EventListenerLoopThread");
        return;
    }

    std::vector<HANDLE> hEvents(numEvents);
    for (int i = 0; i < numEvents; ++i)
    {
        hEvents[i] = OpenEventW(SYNCHRONIZE, FALSE, FANY_IME_EVENT_ARRAY[i].c_str());
        if (!hEvents[i])
        {
            spdlog::error("Failed to open event: {}", wstring_to_string(FANY_IME_EVENT_ARRAY[i]));
            for (int j = 0; j < i; ++j)
            {
                CloseHandle(hEvents[j]);
            }
            return;
        }
    }

    while (true)
    {
        DWORD result = WaitForMultipleObjects(numEvents, hEvents.data(), FALSE, INFINITE);
        if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + numEvents)
        {
            int eventIndex = result - WAIT_OBJECT_0;
            spdlog::info("EventLoopThread: Event {} ({}) triggered!", eventIndex + 1,
                         wstring_to_string(FANY_IME_EVENT_ARRAY[eventIndex]));

            // FanyImeKeyEvent
            if (eventIndex == 0)
            {
            }

            // FanyHideCandidateWndEvent
            if (eventIndex == 1)
            {
                spdlog::info("Hide window!");
                PostMessage(::global_hwnd, WM_HIDE_MAIN_WINDOW, 0, 0);
            }

            // FanyShowCandidateWndEvent
            if (eventIndex == 2)
            {
                spdlog::info("Show window!");
                PostMessage(::global_hwnd, WM_SHOW_MAIN_WINDOW, 0, 0);
            }
        }
        else
        {
            spdlog::error("WaitForMultipleObjects failed with error: {}", GetLastError());
            break;
        }
    }

    for (auto h : hEvents)
    {
        CloseHandle(h);
    }
}
#pragma once

#include "raylib.h"

#include <algorithm>
#include <functional>
#include <vector>

class Timer
{
public:

    using Callback = std::function<void()>;

    // Add a repeating timer
    void start(float intervalSec, Callback cb, bool singleShot = false)
    {
        m_tasks.push_back({ GetTime() + intervalSec, intervalSec, std::move(cb),
                            singleShot });
    }

    // Convenience wrapper: Qt-like singleShot
    void singleShot(float delaySec, Callback cb)
    {
        start(delaySec, std::move(cb), true);
    }

    // Call once per frame
    void update()
    {
        double now = GetTime();
        for (auto it = m_tasks.begin(); it != m_tasks.end();)
        {
            if (now >= it->expiry)
            {
                it->cb();

                if (it->singleShot)
                {
                    it = m_tasks.erase(it); // remove finished
                }
                else
                {
                    it->expiry = now + it->interval; // reschedule
                    ++it;
                }
            }
            else { ++it; }
        }
    }

private:

    struct Task
    {
        double expiry;   // next trigger time
        double interval; // repeat interval
        Callback cb;     // callback to run
        bool singleShot; // fire once?
    };

    std::vector<Task> m_tasks;
};

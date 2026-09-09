import sys

# Replace atomic back to bool but make it volatile.
# On PSP kernel threads, volatile is usually sufficient since it's a simple flag checked once per frame.

def patch(file, s_old, s_new):
    with open(file, 'r') as f:
        c = f.read()
    c = c.replace(s_old, s_new)
    with open(file, 'w') as f:
        f.write(c)

patch('src/main.cpp', '#include <atomic>\nstd::atomic<bool> g_shouldExit{false};', 'volatile bool g_shouldExit = false;')
patch('src/Micro.cpp', '#include <atomic>\nextern std::atomic<bool> g_shouldExit;', 'extern volatile bool g_shouldExit;')
patch('src/MenuManager.cpp', '#include <atomic>\n                extern std::atomic<bool> g_shouldExit;', 'extern volatile bool g_shouldExit;')

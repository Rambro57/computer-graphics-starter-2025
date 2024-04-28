#pragma once

#include "World.hpp"
#include "Window.hpp"

namespace Canis
{
class Editor {
public:
    Editor(Window *_window, World *_world);
    void Draw();
private:
    Window *m_window;
    World *m_world;
    bool showExtra = true;
    int m_index = 0;
};
} // end of Canis namespace
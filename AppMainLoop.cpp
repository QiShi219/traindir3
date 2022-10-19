#include <stdio.h>
#include "HostThread.h"
#include "ClientConnection.h"

extern  "C" void    *AppMainEntryPoint(void *data);
void    RenderState();

ThreadFunction appMainEntryPoint = AppMainEntryPoint;

int     mainLoopIteration;

class AppMainLoop : public HostThread
{
public:
        AppMainLoop();
        ~AppMainLoop();

        void    Run();

        volatile bool    _quit;
};

void    *AppMainEntryPoint(void *data)
{
        AppMainLoop *obj = (AppMainLoop *)data;
        obj->Run();
        return 0;
}


AppMainLoop::AppMainLoop()
: HostThread(AppMainEntryPoint, this)
{
}


void    AppMainLoop::Run()
{
        _quit = false;
        while(!_quit) {
            ++mainLoopIteration;
            //printf("Iteration %d - releasing all connections\n", mainLoopIteration);
            RenderState();
            ReleaseConnections();
            //printf("Iteration %d - sleeping\n", mainLoopIteration);
            Sleep(1000);
        }
}

AppMainLoop *mainLoop;

void    StartApp()
{
        mainLoop = new AppMainLoop();
}

int     count;
char    renderedState[32768];

void    RenderState()
{
        sprintf(renderedState, "assets:\n- id: 1\n  x: 4\n  y: 9\n  type: 'txt'\n  value: '%d'\n", count);
        ++count;
}

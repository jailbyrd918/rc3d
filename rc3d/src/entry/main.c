/*
        DESCRIPTION:
                Entry point of the engine program
*/

#include "core/engine.h"


int main
(int argc, int argv) 
{
        engine_init();
        engine_run();
        engine_quit();

        return 0;
}


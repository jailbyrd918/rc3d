/*
        DESCRIPTION:
                Core of the engine
                Responsible for starting up, running and shutting down the engine
                Responsible for initializing/destroying components, allocating/deallocating resources
                Responsible for calling all the functions required to run the engine
*/

#ifndef ENGINE_H
#define ENGINE_H


// >> starts up the engine
// >> initializes engine components/subsystems, allocates resources
// >> ensures that the engine is in ready state to run
void engine_init
(void);

// >> runs the engine through engine loop
// >> processes input events, updates engine logic, produces output like graphics, audio, etc. 
void engine_run
(void);

// >> destroys engine components/subsystems, deallocates resources
// >> shuts down the engine
void engine_quit
(void);


#endif

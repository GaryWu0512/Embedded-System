#include <stdexcept>
#include <iostream>
#include "elma.h"

namespace elma {

    //! Add a Process to the manager, to be run at a certain frequency.
    //! \param process The process to be scheduled, usually derived from the Process abstract base class
    //! \param period The desired duration of time between updates
    //! \return A reference to the manager, for chaining
    Manager& Manager::schedule(
        Process& process, 
        high_resolution_clock::duration period) {

        process._period = period;
        _processes.push_back(&process); 
        process._manager_ptr = this;            

        return *this;

    }

    //! Add a channel to the manager
    //! \param The channel to be added
    //! \return A reference to the manager, for chaining
    Manager& Manager::add_channel(Channel& channel) {
        _channels[channel.name()] = &channel;
        return *this;
    }

    //! Retrieve a reference to an existing channel. Throws an error if no such channel exists.
    //! \return The channel requested.
    Channel& Manager::channel(string name) {
        if ( _channels.find(name) != _channels.end() ) {
          return *(_channels[name]);
        } else {
            throw Exception("Tried to access an unregistered or non-existant channel.");
        }
    }    

    //! Watch for an event associated with the given name.
    //! For watching events, you would typically register event handlers in your process'
    //! init() method. For example,
    //! @code
    //!     watch("velocity", [this](Event& e) {
    //!         std::cout << "got velocity " << e.value() << std::endl;
    //!     })
    //! @endcode
    //! \param event_name The name of the event
    //! \handler A function or lambda that takes an event and returns nothing.
    Manager& Manager::watch(std::string event_name, std::function<void(Event&)> handler) {
        event_handlers[event_name].push_back(handler);
        return *this;
    }

    Manager& Manager::watch(std::string event_name, std::function<void(Event&)> handler, int priority ) {
        event_handlers[event_name].push_back(handler);
        _priority[event_name].push_back(priority);
        return *this;
    }

    //! Emit an event associated with a name.
    //! Typically, a process would emit events in its update() method using something like
    //! the following code"
    //! @code
    //!     emit(Event("name", value));
    //! @endcode
    //! where value is any jsonable value. For example, you can write
    //! @code
    //!     emit(Event("velocity", 3.41));
    //! @endcode
    //! \param event The Event to be emitted
    //! \return A reference to the manager for chaining.
    Manager& Manager::emit(const Event& event) {
        Event e = event; // make a copy so we can change propagation
        if ( event_handlers.find(event.name()) != event_handlers.end() ) {
            auto pr = _priority[event.name()];
            std::cout<<pr.size();
            if (pr.size()>0){
                std::cout<<"prprprpprprprprprpr";
                auto temp = pr[0];
                //auto handler = event_handlers[event.name()];
                auto temp2 = event_handlers[event.name()][0];

                for (int i = 0; i < pr.size()-1; i++) {
                    for (int j = 0; j < pr.size() -1-i; j++) {
                        //std::cout<<j<<"\n";
                        if (pr[j] > pr[j + 1]) {
                            temp = pr[j + 1];
                            pr[j + 1] = pr[j];
                            pr[j] = temp;
                            temp2= event_handlers[event.name()][j+1];
                            event_handlers[event.name()][j+1]=event_handlers[event.name()][j];
                            event_handlers[event.name()][j]=temp2;
                        }
                    }
                }
            }
            for (int i= 0; i<pr.size();i++){
                std::cout<< pr[i];
            }
            for ( auto handler : event_handlers[event.name()] ) {
                if ( e.propagate() ) {
                  handler(e);
                }
            }
        }
        return *this;
    }

    //! Apply a function to all processes.
    //! \param f The function to apply. It should take a reference to a process and return void.
    //! \return A reference to the manager, for chaining
    Manager& Manager::all(std::function< void(Process&) > f) {
        for(auto process_ptr : _processes) {
            f(*process_ptr);
        }
        return *this;
    }

    //! Initialize all processes. Usually called before run()
    //! \return A reference to the manager, for chaining
    Manager& Manager::init() {
        return all([](Process& p) { p._init();});
    }

    //! Start all processes. Usually not called directly.
    //! \return A reference to the manager, for chaining
    Manager& Manager::start() {
        return all([this](Process& p) { p._start(_elapsed) ;});
    }    

    //! Stop all processes. Usually not called directly.
    //! \return A reference to the manager, for chaining
    Manager& Manager::stop() {
        return all([](Process& p) { p._stop(); });
    }    

    //! Update all processes if enough time has passed. Usually not called directly.
    //! \return A reference to the manager, for chaining
    Manager& Manager::update() { 
        return all([this](Process& p) {
            if ( _elapsed > p.last_update() + p.period() ) {
                p._update(_elapsed);
            }
        });
    }

    //! Run the manager for the specified amount of time.
    //! \param The desired amount of time to run
    //! \return A reference to the manager, for chaining
    Manager& Manager::run(high_resolution_clock::duration runtime) {

        _start_time = high_resolution_clock::now();
        _elapsed = high_resolution_clock::duration::zero();
        start();        

        while ( _elapsed < runtime ) {
            update();
            _elapsed = high_resolution_clock::now() - _start_time;
        }

        stop();

        return *this;

    }

}
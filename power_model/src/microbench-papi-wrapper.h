#pragma once

#include <papi.h>
#include <list>
#include <iostream>

namespace mb{
    enum Target {AMD, NVIDIA, INTEL}; 

    class PapiWrapper{
        public:
            PapiWrapper(Target target, std::list<int> device_ids = {0}, int event_set = 0);
            PapiWrapper();

            void Start();
            void Stop();
            void Print();

            std::string GetCsvHeader();
            std::string GetCsvLine();

            void ConfigureDevices(std::list<int> device_ids);

        private:
            Target target;
            
            int host_eventset;
            std::list<int> host_events;
            long_long *host_counters;

            int device_eventset;
            std::list<std::string> device_events;
            long_long *device_counters;

            std::list<std::list<std::string>> device_event_sets;
            int event_set_selection;
            std::list<int> devices;

            void initAmd();
            void initPapi();

            void initHostEventset();
            void initDeviceEventset();

            void handleReturn(int retval);            

            std::string getEventName(int event);
    };
}

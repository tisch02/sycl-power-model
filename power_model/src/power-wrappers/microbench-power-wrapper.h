#pragma once

#include <iostream>
#include <list>
#include <thread>

namespace mb{
    class PowerWrapper{
        public:
            PowerWrapper();
            PowerWrapper(int interval);

            void Start();
            void Stop();
            void Print();

            std::string GetCsvHeader();
            std::string GetCsvLine();
            void WritePowerCsv(std::string path);

        private:
            int device_count;            
            float measurement_duration;
            uint64_t* power_measurement;
            float* energy_measurement;
            
            std::thread loop_thread;
            bool loop_cancel;
            int loop_interval;        
            std::list<uint64_t> *loop_power_values;
            std::list<long> loop_timestamp_values;

            void measurePower();
            void addMeasurement();
            void handleReturn(int retval);
            void calculateDuration();
            void calculateEnergy();

            void loop();
            int loopCallback();
    };
}
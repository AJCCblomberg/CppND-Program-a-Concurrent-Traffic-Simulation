#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : DONE
    // The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> ulck(_mutex);
    _condition.wait(ulck, [this]{return !_queue.empty();});
    T newMessage = std::move(_queue.back());
    _queue.pop_back();

    return newMessage;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : DONE
    // The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : DONE
    // Add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true) 
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (_queue.receive() == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : DONE
    // Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : DONE 
    // Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    
    // set-up random number generator
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> cycleDurationRange(4000, 6000);
    // set cycle duration
    double cycleDuration = cycleDurationRange(mt);
    
    // initialize cycle start and end time
    std::chrono::time_point<std::chrono::system_clock> cycleTimeStart, cycleTimeEnd;
    // get cycle start time
    cycleTimeStart = std::chrono::high_resolution_clock::now();

    // infinite loop
    while (true) 
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // get cycel end time
        cycleTimeEnd = std::chrono::high_resolution_clock::now();

        // determine time between two loop cycles
        double cycleTime = std::chrono::duration_cast<std::chrono::milliseconds>(cycleTimeEnd - cycleTimeStart).count();

        if (cycleTime >= cycleDuration) {
            // toggle current traffic light phase
            (_currentPhase == TrafficLightPhase::green) ? _currentPhase = TrafficLightPhase::red : _currentPhase = TrafficLightPhase::green;
            
            // send upate to the message _gueue
            TrafficLightPhase update = TrafficLight::getCurrentPhase();
            _queue.send(std::move(update));

            // set cycle duration
            cycleDuration = cycleDurationRange(mt);
            // update cycle start time
            cycleTimeStart = std::chrono::high_resolution_clock::now();
        }
    }
}
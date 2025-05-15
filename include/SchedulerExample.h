#pragma once

#include "GameEvents.h"
#include "Scheduler.h"
#include "SchedulerUtils.h"
#include "TimedEventScheduler.h"
#include "entt/entt.hpp"
#include <iostream>

class SchedulerExample {
public:
  // Run the EntityScheduler example
  void runSchedulerExample();

  // Run the TimedEventScheduler example
  void runTimedEventExample();

  // Run the Event Integration example
  void runEventIntegrationExample();

  // Run all examples
  void run();
};
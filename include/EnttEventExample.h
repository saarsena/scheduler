#pragma once

#include "GameEvents.h"
#include "entt/entt.hpp"
#include <iostream>

class EnttEventExample {
private:
  // Event handler functions
  static void onEntityAttack(const GameEvents::EntityAttackEvent &event);
  static void onEntityDamaged(const GameEvents::EntityDamagedEvent &event);
  static void onEntityDied(const GameEvents::EntityDiedEvent &event);
  static void onPlayerMove(const GameEvents::PlayerMoveEvent &event);
  static void onMapChange(const GameEvents::MapChangeEvent &event);

public:
  // Example integrating EnTT dispatcher
  void runEnttDispatcherExample();

  // Example of combining EnTT dispatcher with the Scheduler
  void runCombinedExample();

  // Run all examples
  void run();
};
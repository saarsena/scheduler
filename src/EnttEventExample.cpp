#include "../include/EnttEventExample.h"

// Event handler functions
void EnttEventExample::onEntityAttack(
    const GameEvents::EntityAttackEvent &event) {
  std::cout << "Attack event: Entity " << static_cast<int>(event.attacker)
            << " attacks " << static_cast<int>(event.target) << " for "
            << event.damage << " damage";

  if (event.critical) {
    std::cout << " (CRITICAL HIT!)";
  }

  std::cout << std::endl;
}

void EnttEventExample::onEntityDamaged(
    const GameEvents::EntityDamagedEvent &event) {
  std::cout << "Damage event: Entity " << static_cast<int>(event.entity)
            << " takes " << event.damage << " damage of type "
            << event.damageType;

  if (event.source != entt::null) {
    std::cout << " from entity " << static_cast<int>(event.source);
  }

  std::cout << std::endl;
}

void EnttEventExample::onEntityDied(const GameEvents::EntityDiedEvent &event) {
  std::cout << "Death event: Entity " << static_cast<int>(event.entity)
            << " died";

  if (event.killer != entt::null) {
    std::cout << " killed by entity " << static_cast<int>(event.killer);
  }

  std::cout << std::endl;
}

void EnttEventExample::onPlayerMove(const GameEvents::PlayerMoveEvent &event) {
  std::cout << "Player " << static_cast<int>(event.player) << " moved from ("
            << event.fromX << "," << event.fromY << ") to (" << event.toX << ","
            << event.toY << ")" << std::endl;
}

void EnttEventExample::onMapChange(const GameEvents::MapChangeEvent &event) {
  std::cout << "Map changed to " << event.mapName;

  if (event.isReload) {
    std::cout << " (reloaded)";
  }

  std::cout << std::endl;
}

// Example integrating EnTT dispatcher
void EnttEventExample::runEnttDispatcherExample() {
  std::cout << "=== EnTT Event Dispatcher Example ===" << std::endl;

  // Create registry and dispatcher
  entt::registry registry;
  entt::dispatcher dispatcher;

  // Connect event listeners
  dispatcher.sink<GameEvents::EntityAttackEvent>()
      .connect<&EnttEventExample::onEntityAttack>();
  dispatcher.sink<GameEvents::EntityDamagedEvent>()
      .connect<&EnttEventExample::onEntityDamaged>();
  dispatcher.sink<GameEvents::EntityDiedEvent>()
      .connect<&EnttEventExample::onEntityDied>();
  dispatcher.sink<GameEvents::PlayerMoveEvent>()
      .connect<&EnttEventExample::onPlayerMove>();
  dispatcher.sink<GameEvents::MapChangeEvent>()
      .connect<&EnttEventExample::onMapChange>();

  // Create some entities
  auto player = registry.create();
  auto enemy = registry.create();

  // Trigger some events

  // Player moves
  dispatcher.enqueue<GameEvents::PlayerMoveEvent>(player, 10, 10, 11, 11);

  // Player attacks enemy
  dispatcher.enqueue<GameEvents::EntityAttackEvent>(player, enemy, 15, false);

  // Enemy takes damage
  dispatcher.enqueue<GameEvents::EntityDamagedEvent>(enemy, 15, player,
                                                     "physical");

  // Player critical attack
  dispatcher.enqueue<GameEvents::EntityAttackEvent>(player, enemy, 30, true);

  // Enemy takes more damage
  dispatcher.enqueue<GameEvents::EntityDamagedEvent>(enemy, 30, player,
                                                     "physical");

  // Enemy dies
  dispatcher.enqueue<GameEvents::EntityDiedEvent>(enemy, player);

  // Map change
  dispatcher.enqueue<GameEvents::MapChangeEvent>("dungeon_level_2", false);

  // Map reload
  dispatcher.enqueue<GameEvents::MapChangeEvent>("dungeon_level_2", true);

  // Process all events
  dispatcher.update();
}

// Example of combining EnTT dispatcher with the Scheduler
void EnttEventExample::runCombinedExample() {
  std::cout << "\n=== Combined Scheduler and Event Dispatcher Example ==="
            << std::endl;

  // Create registry and dispatcher
  entt::registry registry;
  entt::dispatcher dispatcher;

  // This would typically be in the game update loop
  auto tick = 1;

  // Create entities
  auto player = registry.create();
  auto enemy = registry.create();

  // Connect event listeners
  dispatcher.sink<GameEvents::EntityAttackEvent>()
      .connect<&EnttEventExample::onEntityAttack>();
  dispatcher.sink<GameEvents::EntityDamagedEvent>()
      .connect<&EnttEventExample::onEntityDamaged>();
  dispatcher.sink<GameEvents::EntityDiedEvent>()
      .connect<&EnttEventExample::onEntityDied>();
  dispatcher.sink<GameEvents::PlayerMoveEvent>()
      .connect<&EnttEventExample::onPlayerMove>();

  // When player moves, dispatch event
  GameEvents::PlayerMoveEvent moveEvent{player, 5, 5, 6, 6};
  dispatcher.enqueue(moveEvent);

  // When combat occurs:
  // 1. Trigger attack event
  dispatcher.enqueue<GameEvents::EntityAttackEvent>(player, enemy, 25, false);

  // 2. Apply damage and trigger damage event
  dispatcher.enqueue<GameEvents::EntityDamagedEvent>(enemy, 25, player,
                                                     "physical");

  // 3. Check if enemy should die, and if so, trigger death event
  dispatcher.enqueue<GameEvents::EntityDiedEvent>(enemy, player);

  // Process all events
  dispatcher.update();

  std::cout << "Events for tick " << tick << " processed" << std::endl;
}

// Run all examples
void EnttEventExample::run() {
  runEnttDispatcherExample();
  runCombinedExample();
}
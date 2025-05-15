#include "../include/SchedulerExample.h"

void SchedulerExample::runSchedulerExample() {
  std::cout << "=== Scheduler System Example ===" << std::endl;

  // Create registry and scheduler
  entt::registry registry;
  Scheduler scheduler;

  // Create a player entity with health
  auto player = registry.create();
  registry.emplace<Health>(player, 100, 100);

  // Create an enemy entity with health
  auto enemy = registry.create();
  registry.emplace<Health>(enemy, 50, 50);

  // Print entity health
  auto printHealth = [&](const char *name, entt::entity entity) {
    const auto &health = registry.get<Health>(entity);
    std::cout << name << " health: " << health.current << "/" << health.max
              << std::endl;
  };

  // Print initial health
  printHealth("Player", player);
  printHealth("Enemy", enemy);

  // Schedule a basic attack from enemy to player at tick 5
  scheduler.schedule(
      5, enemy, [player](entt::entity attacker, entt::registry &reg) {
        if (reg.valid(attacker) && reg.valid(player)) {
          auto &playerHealth = reg.get<Health>(player);
          playerHealth.current -= 10;
          std::cout << "Enemy attacks player for 10 damage!" << std::endl;
        }
      });

  // Use utility to schedule player attacks on enemy at ticks 3 and 7
  SchedulerUtils::scheduleAttack(scheduler, player, enemy, 15, 3,
                                 [](entt::entity, entt::entity, int damage) {
                                   std::cout << "Player attacks enemy for "
                                             << damage << " damage!"
                                             << std::endl;
                                 });

  // Schedule an attack with an onComplete callback
  scheduler.schedule(
      7, player,
      // Main action
      [enemy](entt::entity attacker, entt::registry &reg) {
        if (reg.valid(attacker) && reg.valid(enemy)) {
          auto &enemyHealth = reg.get<Health>(enemy);
          enemyHealth.current -= 20;
          std::cout << "Player attacks enemy for 20 damage!" << std::endl;
        }
      },
      // onComplete callback
      [enemy](ActionID id, entt::entity attacker, entt::registry &reg,
              entt::dispatcher &disp) {
        if (reg.valid(attacker) && reg.valid(enemy)) {
          // Check if enemy health is low and enqueue a follow-up event
          auto &enemyHealth = reg.get<Health>(enemy);
          if (enemyHealth.current < 20) {
            std::cout << "Enemy is critically wounded!" << std::endl;
            disp.enqueue<GameEvents::CombatEndEvent>(attacker, false);
          }
        }
      });

  // Schedule poison damage over time on the player
  SchedulerUtils::scheduleDamageOverTime(
      scheduler, player, 5, 3, 2, 4, [](entt::entity, int damage) {
        std::cout << "Player takes " << damage << " poison damage!"
                  << std::endl;
      });

  // Run the simulation for 10 ticks
  for (int tick = 1; tick <= 10; ++tick) {
    std::cout << "\n-- Tick " << tick << " --" << std::endl;

    // Create a dispatcher to handle action completion events
    entt::dispatcher dispatcher;

    // Connect a listener for ActionCompletedEvent
    // Create named lambdas
      const auto actionCompletedHandler = [](const GameEvents::ActionCompletedEvent& event) {
          std::cout << "Action " << event.actionId << " completed for entity "
                    << static_cast<int>(event.entity) << std::endl;
      };

      const auto combatEndHandler = [](const GameEvents::CombatEndEvent& event) {
          std::cout << "Combat ended! "
                    << (event.fled
                        ? "Someone fled."
                        : ("Winner is entity " +
                           std::to_string(static_cast<int>(event.winner))))
                    << std::endl;
      };

      // Connect using the instance of the lambda (auto doesn't work here)
      dispatcher.sink<GameEvents::ActionCompletedEvent>().connect<&decltype(actionCompletedHandler)::operator()>(actionCompletedHandler);
      dispatcher.sink<GameEvents::CombatEndEvent>().connect<&decltype(combatEndHandler)::operator()>(combatEndHandler);

    // Update the scheduler with the dispatcher
    scheduler.update(tick, registry, dispatcher);

    // Process all triggered events
    dispatcher.update();

    // Print health after each tick
    if (registry.valid(player)) {
      printHealth("Player", player);
    }

    if (registry.valid(enemy)) {
      printHealth("Enemy", enemy);
    }
  }
}

void SchedulerExample::runTimedEventExample() {
  std::cout << "\n=== Timed Event System Example ===" << std::endl;

  TimedEventScheduler eventScheduler;

  // Schedule some simple function events
  eventScheduler.scheduleFunction(
      3, []() { std::cout << "Function event at tick 3" << std::endl; });

  eventScheduler.scheduleFunction(
      5, []() { std::cout << "Function event at tick 5" << std::endl; });

  // Create a custom event
  class GameStartEvent : public TimedEvent {
  public:
    GameStartEvent(int tick) : TimedEvent(tick, "GameStart") {}

    void execute() override {
      std::cout << "Game starts! Initializing systems..." << std::endl;
    }
  };

  // Schedule the custom event
  eventScheduler.scheduleEvent<GameStartEvent>(1);

  // Create a cancellable event
  auto eventId = eventScheduler.scheduleFunction(
      4, []() { std::cout << "This event will be cancelled" << std::endl; });

  // Cancel the event
  eventScheduler.cancelEvent(eventId);

  // Run the simulation for 6 ticks
  for (int tick = 1; tick <= 6; ++tick) {
    std::cout << "\n-- Tick " << tick << " --" << std::endl;
    eventScheduler.update(tick);
  }
}

void SchedulerExample::runEventIntegrationExample() {
  std::cout << "\n=== Scheduler Event Integration Example ===" << std::endl;

  // Create registry, scheduler, and dispatcher
  entt::registry registry;
  Scheduler scheduler;
  entt::dispatcher dispatcher;

  // Connect event handlers
// ActionCompletedEvent handler with connect_arg
    const auto actionCompletedHandler = [](const auto actionId, const auto& event) {
        std::cout << "Action " << actionId << " completed for entity "
                  << static_cast<int>(event.entity) << std::endl;
    };

// Define function object classes
    struct ActionCompletedHandler {
        void operator()(const GameEvents::ActionCompletedEvent& event) const {
            std::cout << "Action " << event.actionId << " completed for entity "
                      << static_cast<int>(event.entity) << std::endl;
        }
    };

    struct EntityDamagedHandler {
        void operator()(const GameEvents::EntityDamagedEvent& event) const {
            std::cout << "Entity " << static_cast<int>(event.entity)
                      << " damaged for " << event.damage << " points!" << std::endl;
        }
    };

// Create instances and connect
    ActionCompletedHandler actionHandler;
    EntityDamagedHandler damageHandler;

    dispatcher.sink<GameEvents::ActionCompletedEvent>()
            .connect<&ActionCompletedHandler::operator()>(actionHandler);

    dispatcher.sink<GameEvents::EntityDamagedEvent>()
            .connect<&EntityDamagedHandler::operator()>(damageHandler);  // Create entities
  auto player = registry.create();
  registry.emplace<Health>(player, 100, 100);

  auto enemy = registry.create();
  registry.emplace<Health>(enemy, 50, 50);

  // Schedule an action with event publication via onComplete
  scheduler.schedule(
      2, player,
      // Main action
      [enemy](entt::entity attacker, entt::registry &reg) {
        // Apply damage
        auto &health = reg.get<Health>(enemy);
        health.current -= 15;
        std::cout << "Player strikes enemy for 15 damage" << std::endl;
      },
      // onComplete callback
      [enemy](ActionID id, entt::entity attacker, entt::registry &reg,
              entt::dispatcher &disp) {
        // Publish an event about the damage
        disp.enqueue<GameEvents::EntityDamagedEvent>(enemy,    // entity damaged
                                                     15,       // damage amount
                                                     attacker, // source
                                                     "physical" // damage type
        );
      });

  // Run for 3 ticks
  for (int tick = 1; tick <= 3; ++tick) {
    std::cout << "\n-- Tick " << tick << " --" << std::endl;

    // Update scheduler with dispatcher
    scheduler.update(tick, registry, dispatcher);

    // Process all events
    dispatcher.update();
  }
}

void SchedulerExample::run() {
  runSchedulerExample();
  runTimedEventExample();
  runEventIntegrationExample();
}
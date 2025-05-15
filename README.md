# Game Event Scheduler

A flexible and lightweight event scheduling system for C++ game development, designed to work with EnTT entity-component framework.

## Overview

This scheduler provides a robust system for managing time-based events and actions in game environments. It allows developers to schedule entity actions, delayed events, recurring events, and complex sequences with minimal overhead.

## Features

- Schedule entity actions at specific ticks/timestamps
- Support for callbacks on action completion
- Integration with EnTT registry and dispatcher
- Damage-over-time and recurring event utilities
- Custom event system with cancellation support
- Clean API for game event handling

## Requirements

- C++17 or newer
- EnTT 3.x

## Installation

This library is header-only. Simply copy the include directory to your project.

```cpp
#include "Scheduler.h"
#include "TimedEventScheduler.h"
```

## Usage Examples

### Basic Scheduling

```cpp
// Create registry and scheduler
entt::registry registry;
Scheduler scheduler;

// Schedule an action at tick 5
scheduler.schedule(5, entity, [](entt::entity entity, entt::registry& reg) {
    // Action to perform
    auto& health = reg.get<Health>(entity);
    health.current -= 10;
});

// Update the scheduler on each game tick
for (int tick = 1; tick <= 10; ++tick) {
    entt::dispatcher dispatcher;
    scheduler.update(tick, registry, dispatcher);
    dispatcher.update();
}
```

### With Completion Callbacks

```cpp
scheduler.schedule(
    7, player,
    // Main action
    [enemy](entt::entity attacker, entt::registry& reg) {
        auto& enemyHealth = reg.get<Health>(enemy);
        enemyHealth.current -= 20;
    },
    // onComplete callback
    [enemy](ActionID id, entt::entity attacker, entt::registry& reg, entt::dispatcher& disp) {
        auto& enemyHealth = reg.get<Health>(enemy);
        if (enemyHealth.current < 20) {
            disp.enqueue<GameEvents::CombatEndEvent>(attacker, false);
        }
    }
);
```

### Utility Functions

```cpp
// Schedule damage over time (DoT)
SchedulerUtils::scheduleDamageOverTime(
    scheduler,    // scheduler instance
    player,       // target entity
    5,            // damage per tick
    3,            // start tick
    2,            // interval between ticks
    4             // number of ticks
);

// Schedule an attack
SchedulerUtils::scheduleAttack(
    scheduler,    // scheduler instance
    player,       // attacker
    enemy,        // target
    15,           // damage amount
    3             // tick to execute
);
```

### Using the TimedEventScheduler

```cpp
TimedEventScheduler eventScheduler;

// Schedule a simple function
eventScheduler.scheduleFunction(3, []() { 
    std::cout << "Event at tick 3" << std::endl; 
});

// Schedule a custom event
class GameStartEvent : public TimedEvent {
public:
    GameStartEvent(int tick) : TimedEvent(tick, "GameStart") {}

    void execute() override {
        std::cout << "Game starts!" << std::endl;
    }
};

eventScheduler.scheduleEvent<GameStartEvent>(1);

// Update the scheduler each tick
for (int tick = 1; tick <= 10; ++tick) {
    eventScheduler.update(tick);
}
```

## Event Integration

The scheduler works seamlessly with EnTT's event dispatcher:

```cpp
// Create registry, scheduler, and dispatcher
entt::registry registry;
Scheduler scheduler;
entt::dispatcher dispatcher;

// Connect event handlers
dispatcher.sink<GameEvents::EntityDamagedEvent>()
    .connect<&onEntityDamaged>();

// Schedule actions that trigger events
scheduler.schedule(
    2, player,
    [enemy](entt::entity attacker, entt::registry& reg) {
        auto& health = reg.get<Health>(enemy);
        health.current -= 15;
    },
    [enemy](ActionID id, entt::entity attacker, entt::registry& reg, entt::dispatcher& disp) {
        // Publish an event about the damage
        disp.enqueue<GameEvents::EntityDamagedEvent>(
            enemy,      // entity damaged
            15,         // damage amount
            attacker,   // source
            "physical"  // damage type
        );
    }
);

// Update both systems
scheduler.update(tick, registry, dispatcher);
dispatcher.update();
```

## License

[MIT License](LICENSE)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

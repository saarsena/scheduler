#pragma once

#include "Scheduler.h"
#include "TimedEventScheduler.h"
#include <functional>
#include <vector>

// Simple component for health
struct Health {
  int current = 100;
  int max = 100;
};

namespace SchedulerUtils {

// Schedule damage over time (like poison, burning, etc.)
inline std::vector<ActionID> scheduleDamageOverTime(
    Scheduler &scheduler, entt::entity target, int damage, int totalTicks,
    int interval, int startTick,
    std::function<void(entt::entity, int)> onDamage = nullptr) {

  std::vector<ActionID> actionIds;

  for (int i = 0; i < totalTicks; ++i) {
    int tick = startTick + (i * interval);

    ActionID id = scheduler.schedule(
        tick, target,
        [damage, onDamage](entt::entity entity, entt::registry &registry) {
          if (registry.valid(entity) && registry.all_of<Health>(entity)) {
            auto &health = registry.get<Health>(entity);
            health.current -= damage;

            if (onDamage) {
              onDamage(entity, damage);
            }
          }
        });

    actionIds.push_back(id);
  }

  return actionIds;
}

// Schedule an attack with a callback when done
inline ActionID scheduleAttack(
    Scheduler &scheduler, entt::entity attacker, entt::entity target,
    int damage, int tick,
    std::function<void(entt::entity, entt::entity, int)> onAttack = nullptr) {

  return scheduler.schedule(
      tick, attacker,
      [target, damage, onAttack](entt::entity entity,
                                 entt::registry &registry) {
        if (!registry.valid(entity) || !registry.valid(target)) {
          return;
        }

        if (registry.all_of<Health>(target)) {
          auto &health = registry.get<Health>(target);
          health.current -= damage;

          if (onAttack) {
            onAttack(entity, target, damage);
          }
        }
      });
}

// Schedule a delayed action on an entity
inline ActionID scheduleDelayedAction(
    Scheduler &scheduler, entt::entity entity, int delayTicks, int currentTick,
    std::function<void(entt::entity, entt::registry &)> action) {

  return scheduler.schedule(currentTick + delayTicks, entity,
                            std::move(action));
}

// Schedule a recurring action
inline std::vector<ActionID> scheduleRecurringAction(
    Scheduler &scheduler, entt::entity entity, int interval, int count,
    int startTick, std::function<void(entt::entity, entt::registry &)> action) {

  std::vector<ActionID> actionIds;

  for (int i = 0; i < count; ++i) {
    int tick = startTick + (i * interval);
    ActionID id = scheduler.schedule(tick, entity, action);
    actionIds.push_back(id);
  }

  return actionIds;
}

// Schedule an action chain (one after another)
inline std::vector<ActionID> scheduleActionChain(
    Scheduler &scheduler, entt::entity entity,
    std::vector<
        std::pair<int, std::function<void(entt::entity, entt::registry &)>>>
        actions) {

  std::vector<ActionID> actionIds;

  for (const auto &[delay, action] : actions) {
    ActionID id = scheduler.schedule(delay, entity, action);
    actionIds.push_back(id);
  }

  return actionIds;
}

} // namespace SchedulerUtils
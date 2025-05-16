/**
 * @file Scheduler.h
 * @brief A scheduler for managing and executing entity-based actions at specific time intervals.
 *
 * This class provides functionality to schedule, manage, and execute entity-based actions
 * within an EnTT entity-component system at specific ticks. Actions can be cancelled
 * before execution and can include completion callbacks.
 */
#pragma once

#include "GameEvents.h"
#include "entt/entt.hpp"
#include <functional>
#include <queue>
#include <unordered_set>

/// @typedef ActionID
/// @brief Unique identifier for scheduled actions
using ActionID = uint32_t;

/**
 * @struct ScheduledAction
 * @brief Represents an action scheduled to be executed at a specific tick.
 *
 * A ScheduledAction contains the action to perform on an entity, the tick at which to
 * execute it, and optional completion callback logic.
 */
struct ScheduledAction {
    ActionID id;         ///< Unique identifier for the action
    int tick;            ///< Tick at which to execute the action
    entt::entity entity; ///< Entity on which to perform the action

    /// @brief The main action function to execute
    /// @param entity The entity to operate on
    /// @param registry The EnTT registry containing components
    std::function<void(entt::entity, entt::registry &)> action;

    /// @brief Optional callback called after the action completes
    /// @param id The ID of the completed action
    /// @param entity The entity the action was performed on
    /// @param registry The EnTT registry containing components
    /// @param dispatcher The EnTT event dispatcher for emitting events
    std::function<void(ActionID, entt::entity, entt::registry &, entt::dispatcher &)> onComplete;

    /**
     * @brief Comparison operator for priority queue ordering
     * @param other Other ScheduledAction to compare against
     * @return true if this action should be processed after the other action
     *
     * Actions are ordered by tick, with earlier ticks having higher priority.
     */
    bool operator<(const ScheduledAction &other) const {
        return tick > other.tick; // Min-heap behavior for priority queue
    }
};

/**
 * @class Scheduler
 * @brief Manages and executes time-based actions on entities within an EnTT framework.
 *
 * The Scheduler maintains a queue of actions to be executed at specific ticks,
 * operating on entities within an EnTT registry. It supports scheduling, cancelling,
 * and executing actions with completion callbacks and event dispatching.
 *
 * @code
 * // Example usage:
 * entt::registry registry;
 * entt::dispatcher dispatcher;
 * Scheduler scheduler;
 *
 * // Create an entity
 * auto entity = registry.create();
 * registry.emplace<Position>(entity, 0.0f, 0.0f);
 *
 * // Schedule an action to move the entity at tick 100
 * scheduler.schedule(100, entity,
 *   [](entt::entity e, entt::registry &r) {
 *     auto &pos = r.get<Position>(e);
 *     pos.x += 10.0f;
 *     pos.y += 5.0f;
 *   }
 * );
 *
 * // Process actions for the current tick
 * scheduler.update(currentTick, registry, dispatcher);
 * @endcode
 */
class Scheduler {
  public:
    /**
     * @brief Constructs a new Scheduler
     */
    Scheduler() : nextActionId(1) {}

    /**
     * @brief Schedule an action and get its ID
     * @param action The ScheduledAction to schedule
     * @return The ID of the scheduled action, which can be used for cancellation
     *
     * Takes a pre-constructed ScheduledAction and adds it to the queue.
     */
    ActionID schedule(const ScheduledAction &action) {
        ActionID actionId = nextActionId++;
        ScheduledAction actionWithId = action;
        actionWithId.id = actionId;
        queue.push(actionWithId);
        activeActions.insert(actionId);
        return actionId;
    }

    /**
     * @brief Convenience method to create and schedule an action
     * @param tick The tick at which to execute the action
     * @param entity The entity on which to perform the action
     * @param action The function to execute on the entity
     * @param onComplete Optional callback when the action completes
     * @return The ID of the scheduled action
     *
     * Creates a ScheduledAction from the provided parameters and schedules it.
     *
     * @code
     * // Example: Schedule an entity to be damaged after 5 ticks
     * scheduler.schedule(currentTick + 5, enemy,
     *   [](entt::entity e, entt::registry &r) {
     *     auto &health = r.get<Health>(e);
     *     health.value -= 10;
     *   },
     *   [](ActionID id, entt::entity e, entt::registry &r, entt::dispatcher &d) {
     *     // Check if entity died from the damage
     *     if (r.get<Health>(e).value <= 0) {
     *       d.enqueue<GameEvents::EntityDiedEvent>(e);
     *     }
     *   }
     * );
     * @endcode
     */
    ActionID
    schedule(int tick, entt::entity entity,
             std::function<void(entt::entity, entt::registry &)> action,
             std::function<void(ActionID, entt::entity, entt::registry &, entt::dispatcher &)>
                 onComplete = nullptr) {
        ScheduledAction scheduledAction{0, tick, entity, std::move(action), std::move(onComplete)};
        return schedule(scheduledAction);
    }

    /**
     * @brief Cancel a scheduled action
     * @param id The ID of the action to cancel
     * @return true if the action was found and cancelled, false otherwise
     *
     * Note: This only marks the action as inactive; it will be skipped when
     * its turn comes up in the action queue.
     */
    bool cancel(ActionID id) {
        if (activeActions.find(id) != activeActions.end()) {
            // We can't remove from the priority queue directly, so we mark it as
            // inactive
            activeActions.erase(id);
            return true;
        }
        return false;
    }

    /**
     * @brief Process all actions scheduled up to the given tick
     * @param current_tick The current system tick
     * @param registry The EnTT registry for component access
     * @param dispatcher The EnTT event dispatcher for emitting events
     *
     * This method executes all actions that are due at or before the current tick.
     * For each action:
     * 1. Checks if the action is still active (not cancelled)
     * 2. Verifies that the target entity still exists
     * 3. Executes the main action function
     * 4. Dispatches a standard completion event
     * 5. Calls the custom onComplete callback if provided
     */
    void update(int current_tick, entt::registry &registry, entt::dispatcher &dispatcher) {
        while (!queue.empty() && queue.top().tick <= current_tick) {
            ScheduledAction action = queue.top();
            queue.pop();

            // Skip if the action was cancelled
            if (activeActions.find(action.id) == activeActions.end()) {
                continue;
            }

            // Execute the action if the entity is still valid
            if (registry.valid(action.entity)) {
                // Execute main action
                action.action(action.entity, registry);

                // Trigger standard completion event
                dispatcher.enqueue<GameEvents::ActionCompletedEvent>(action.id, action.entity);

                // Call custom onComplete if provided
                if (action.onComplete) {
                    action.onComplete(action.id, action.entity, registry, dispatcher);
                }
            }

            // Remove from active actions
            activeActions.erase(action.id);
        }
    }

    /**
     * @brief Clear all pending actions
     *
     * This removes all scheduled actions from the queue and active list.
     * Useful when transitioning between game states or resetting the system.
     */
    void clear() {
        // Clear the queue - we have to rebuild it
        std::priority_queue<ScheduledAction> empty;
        std::swap(queue, empty);
        activeActions.clear();
    }

  private:
    std::priority_queue<ScheduledAction> queue; ///< Queue of pending actions, ordered by tick
    std::unordered_set<ActionID> activeActions; ///< Set of active action IDs
    ActionID nextActionId;                      ///< Next available action ID
};

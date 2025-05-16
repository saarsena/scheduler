/**
 * @file TimedEventScheduler.h
 * @brief A scheduler for managing and executing events at specific time intervals.
 *
 * This class provides functionality to schedule, manage, and execute events at
 * specific ticks (time intervals). Events can be prioritized within the same tick
 * and can be cancelled before execution.
 */
#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_set>

/// @typedef EventID
/// @brief Unique identifier for scheduled events
using EventID = uint32_t;

// Forward declaration of the TimedEventScheduler for the event
class TimedEventScheduler;

/**
 * @class TimedEvent
 * @brief Base class for all timed events in the scheduler system.
 *
 * TimedEvent represents an action that should be executed at a specific tick.
 * Derived classes must implement the execute() method to define the action.
 */
class TimedEvent {
  public:
    /**
     * @brief Constructs a timed event scheduled for a specific tick
     * @param tick The tick at which this event should execute
     * @param name Optional name for identifying the event (default: empty string)
     */
    TimedEvent(int tick, std::string name = "")
        : id(0), tick(tick), name(std::move(name)), priority(0) {}

    /// Virtual destructor for proper inheritance
    virtual ~TimedEvent() = default;

    /**
     * @brief The action to perform when the event triggers
     *
     * This method must be implemented by derived classes to define
     * what happens when the event is executed.
     */
    virtual void execute() = 0;

    /**
     * @brief Sets the parent scheduler for this event
     * @param scheduler Pointer to the scheduler managing this event
     *
     * Allows events to interact with their scheduler, such as scheduling
     * follow-up events from within their execute() method.
     */
    virtual void setScheduler(TimedEventScheduler *scheduler) { this->scheduler = scheduler; }

    /// @brief Gets the tick at which this event is scheduled to execute
    /// @return The scheduled tick
    int getTick() const { return tick; }

    /// @brief Gets the name of this event
    /// @return The event name
    const std::string &getName() const { return name; }

    /// @brief Gets the unique ID of this event
    /// @return The event ID
    EventID getId() const { return id; }

    /// @brief Gets the priority of this event
    /// @return The event priority (higher values execute first within the same tick)
    int getPriority() const { return priority; }

    /// @brief Sets the ID for this event (used by the scheduler)
    /// @param newId The new ID to assign
    void setId(EventID newId) { id = newId; }

    /// @brief Sets the priority for this event
    /// @param newPriority The new priority level (higher values execute first)
    void setPriority(int newPriority) { priority = newPriority; }

  protected:
    /// Pointer to the scheduler that manages this event
    TimedEventScheduler *scheduler = nullptr;

  private:
    EventID id;       ///< Unique identifier
    int tick;         ///< Tick at which to execute
    std::string name; ///< Optional name for the event
    int priority;     ///< Execution priority within the same tick
};

/**
 * @struct TimedEventCompare
 * @brief Comparison functor for ordering events in the priority queue
 *
 * Events are ordered first by tick (earlier ticks first), and then by
 * priority (higher priority first) for events scheduled at the same tick.
 */
struct TimedEventCompare {
    /**
     * @brief Compares two events for priority queue ordering
     * @param a First event to compare
     * @param b Second event to compare
     * @return true if a should be processed after b, false otherwise
     */
    bool operator()(const std::shared_ptr<TimedEvent> &a,
                    const std::shared_ptr<TimedEvent> &b) const {
        if (a->getTick() == b->getTick()) {
            // If same tick, use priority (higher first)
            return a->getPriority() < b->getPriority();
        }
        // Otherwise, sort by tick (lower first)
        return a->getTick() > b->getTick();
    }
};

/**
 * @class FunctionEvent
 * @brief Convenience class for simple function-based events
 *
 * This class wraps a std::function to be executed as an event,
 * simplifying the creation of one-off events without requiring
 * a full class implementation.
 */
class FunctionEvent : public TimedEvent {
  public:
    /**
     * @brief Constructs a function-based event
     * @param tick The tick at which to execute
     * @param func The function to call when executed
     * @param name Optional name for the event
     */
    FunctionEvent(int tick, std::function<void()> func, std::string name = "")
        : TimedEvent(tick, std::move(name)), func(std::move(func)) {}

    /**
     * @brief Executes the wrapped function
     *
     * Called by the scheduler when this event is due.
     */
    void execute() override { func(); }

  private:
    std::function<void()> func; ///< The function to execute
};

/**
 * @class TimedEventScheduler
 * @brief Manages and executes time-based events
 *
 * The TimedEventScheduler maintains a queue of events to be executed at specific
 * ticks. It supports scheduling, cancelling, and executing events with different
 * priorities.
 *
 * @code
 * // Example usage:
 * TimedEventScheduler scheduler;
 *
 * // Schedule a function to run at tick 100
 * scheduler.scheduleFunction(100, []() {
 *   std::cout << "Hello at tick 100!" << std::endl;
 * }, "greeting");
 *
 * // Process events for the current tick
 * scheduler.update(currentTick);
 * @endcode
 */
class TimedEventScheduler {
  public:
    /**
     * @brief Constructs a new event scheduler
     */
    TimedEventScheduler() : nextEventId(1) {}

    /**
     * @brief Schedules a new event of specified type
     * @tparam EventType The type of event to schedule (must derive from TimedEvent)
     * @tparam Args Types of arguments to forward to the EventType constructor
     * @param args Arguments to forward to the EventType constructor
     * @return The ID of the scheduled event
     *
     * @code
     * // Example: Schedule a custom event
     * class MyEvent : public TimedEvent {
     *   public:
     *     MyEvent(int tick, int value) : TimedEvent(tick), value(value) {}
     *     void execute() override { std::cout << "Value: " << value << std::endl; }
     *   private:
     *     int value;
     * };
     *
     * EventID id = scheduler.scheduleEvent<MyEvent>(100, 42);
     * @endcode
     */
    template <typename EventType, typename... Args> EventID scheduleEvent(Args &&...args) {
        auto event = std::make_shared<EventType>(std::forward<Args>(args)...);
        return scheduleEvent(event);
    }

    /**
     * @brief Schedules a pre-created event
     * @param event The event to schedule
     * @return The ID of the scheduled event
     */
    EventID scheduleEvent(std::shared_ptr<TimedEvent> event) {
        EventID eventId = nextEventId++;
        event->setId(eventId);
        event->setScheduler(this);
        eventQueue.push(event);
        activeEvents.insert(eventId);
        return eventId;
    }

    /**
     * @brief Attempts to cancel a scheduled event
     * @param id The ID of the event to cancel
     * @return true if the event was found and cancelled, false otherwise
     *
     * Note: This only marks the event as inactive; it will be skipped when
     * its turn comes up in the event queue.
     */
    bool cancelEvent(EventID id) {
        if (activeEvents.find(id) != activeEvents.end()) {
            activeEvents.erase(id);
            return true;
        }
        return false;
    }

    /**
     * @brief Processes all events scheduled up to the given tick
     * @param currentTick The current system tick
     *
     * This method executes all events that are due at or before the current tick,
     * in order of tick and then priority.
     */
    void update(int currentTick) {
        while (!eventQueue.empty() && eventQueue.top()->getTick() <= currentTick) {
            auto event = eventQueue.top();
            eventQueue.pop();

            // Skip if the event was cancelled
            if (activeEvents.find(event->getId()) == activeEvents.end()) {
                continue;
            }

            // Execute the event
            event->execute();

            // Remove from active events
            activeEvents.erase(event->getId());
        }
    }

    /**
     * @brief Schedules a simple function to run at a specific tick
     * @param tick The tick at which to execute the function
     * @param func The function to execute
     * @param name Optional name for the event
     * @return The ID of the scheduled event
     *
     * This is a convenience method for quickly scheduling function-based events
     * without creating a custom event class.
     */
    EventID scheduleFunction(int tick, std::function<void()> func, std::string name = "") {
        return scheduleEvent<FunctionEvent>(tick, std::move(func), std::move(name));
    }

    /**
     * @brief Clears all pending events
     *
     * This removes all scheduled events from the queue and active list.
     */
    void clear() {
        std::priority_queue<std::shared_ptr<TimedEvent>, std::vector<std::shared_ptr<TimedEvent>>,
                            TimedEventCompare>
            empty;
        std::swap(eventQueue, empty);
        activeEvents.clear();
    }

  private:
    /// Queue of pending events, ordered by tick and priority
    std::priority_queue<std::shared_ptr<TimedEvent>, std::vector<std::shared_ptr<TimedEvent>>,
                        TimedEventCompare>
        eventQueue;

    /// Set of active event IDs (used to check if events have been cancelled)
    std::unordered_set<EventID> activeEvents;

    /// Next available event ID
    EventID nextEventId;
};

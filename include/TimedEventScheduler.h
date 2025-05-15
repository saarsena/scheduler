#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <unordered_set>

using EventID = uint32_t;

// Forward declaration of the TimedEventScheduler for the event
class TimedEventScheduler;

// Base class for all timed events
class TimedEvent {
public:
  TimedEvent(int tick, std::string name = "")
      : id(0), tick(tick), name(std::move(name)), priority(0) {}

  virtual ~TimedEvent() = default;

  // The action to perform when the event triggers
  virtual void execute() = 0;

  // Allow events to schedule follow-up events
  virtual void setScheduler(TimedEventScheduler *scheduler) {
    this->scheduler = scheduler;
  }

  int getTick() const { return tick; }
  const std::string &getName() const { return name; }
  EventID getId() const { return id; }
  int getPriority() const { return priority; }

  void setId(EventID newId) { id = newId; }
  void setPriority(int newPriority) { priority = newPriority; }

protected:
  TimedEventScheduler *scheduler = nullptr;

private:
  EventID id;
  int tick;
  std::string name;
  int priority; // Higher priority events execute first within the same tick
};

// Comparison for the priority queue
struct TimedEventCompare {
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

// Convenience class for simple function-based events
class FunctionEvent : public TimedEvent {
public:
  FunctionEvent(int tick, std::function<void()> func, std::string name = "")
      : TimedEvent(tick, std::move(name)), func(std::move(func)) {}

  void execute() override { func(); }

private:
  std::function<void()> func;
};

class TimedEventScheduler {
public:
  TimedEventScheduler() : nextEventId(1) {}

  // Schedule a TimedEvent and return its ID
  template <typename EventType, typename... Args>
  EventID scheduleEvent(Args &&...args) {
    auto event = std::make_shared<EventType>(std::forward<Args>(args)...);
    return scheduleEvent(event);
  }

  // Schedule an already created event
  EventID scheduleEvent(std::shared_ptr<TimedEvent> event) {
    EventID eventId = nextEventId++;
    event->setId(eventId);
    event->setScheduler(this);
    eventQueue.push(event);
    activeEvents.insert(eventId);
    return eventId;
  }

  // Cancel a scheduled event
  bool cancelEvent(EventID id) {
    if (activeEvents.find(id) != activeEvents.end()) {
      activeEvents.erase(id);
      return true;
    }
    return false;
  }

  // Process events due at the current tick
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

  // Schedule a simple function to run at a specific tick
  EventID scheduleFunction(int tick, std::function<void()> func,
                           std::string name = "") {
    return scheduleEvent<FunctionEvent>(tick, std::move(func), std::move(name));
  }

  // Clear all pending events
  void clear() {
    std::priority_queue<std::shared_ptr<TimedEvent>,
                        std::vector<std::shared_ptr<TimedEvent>>,
                        TimedEventCompare>
        empty;
    std::swap(eventQueue, empty);
    activeEvents.clear();
  }

private:
  std::priority_queue<std::shared_ptr<TimedEvent>,
                      std::vector<std::shared_ptr<TimedEvent>>,
                      TimedEventCompare>
      eventQueue;
  std::unordered_set<EventID> activeEvents;
  EventID nextEventId;
};
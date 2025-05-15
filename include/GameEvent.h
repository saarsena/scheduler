#pragma once

// Define event types
enum class GameEventType {
  PLAYER_MOVED,
  PLAYER_COLLISION,
  ENTITY_SPAWN,
  ENTITY_DESTROY,
  MAP_CHANGED,
  MOB_MOVED
};

// Base event class
class GameEvent {
public:
  explicit GameEvent(GameEventType type) : type(type) {}
  virtual ~GameEvent() = default;

  GameEventType getType() const { return type; }

private:
  GameEventType type;
};

// Player movement event
class PlayerMovedEvent : public GameEvent {
public:
  PlayerMovedEvent(int newX, int newY)
      : GameEvent(GameEventType::PLAYER_MOVED), newX(newX), newY(newY) {}

  int newX;
  int newY;
};

// Mob movement event
class MobMovedEvent : public GameEvent {
public:
  MobMovedEvent(int newX, int newY)
      : GameEvent(GameEventType::MOB_MOVED), newX(newX), newY(newY) {}

  int newX;
  int newY;
};
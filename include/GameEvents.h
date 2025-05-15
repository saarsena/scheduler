#pragma once

#include "entt/entt.hpp"
#include <string>

// Use the same definition as in Scheduler.h
using ActionID = uint32_t;

// EnTT event types for game events
namespace GameEvents {

// Entity attack event
struct EntityAttackEvent {
  entt::entity attacker;
  entt::entity target;
  int damage;
  bool critical;
};

// Entity damaged event
struct EntityDamagedEvent {
  entt::entity entity;
  int damage;
  entt::entity source;    // Optional source of damage
  std::string damageType; // "physical", "poison", "fire", etc.
};

// Entity died event
struct EntityDiedEvent {
  entt::entity entity;
  entt::entity killer; // Optional killer
};

// Spawn entity event
struct EntitySpawnEvent {
  entt::entity entity;
  int x;
  int y;
  std::string entityType;
};

// Map change event
struct MapChangeEvent {
  std::string mapName;
  bool isReload;
};

// Player move event
struct PlayerMoveEvent {
  entt::entity player;
  int fromX;
  int fromY;
  int toX;
  int toY;
};

// Item pickup event
struct ItemPickupEvent {
  entt::entity player;
  entt::entity item;
  std::string itemType;
};

// Combat start/end events
struct CombatStartEvent {
  entt::entity initiator;
  entt::entity target;
};

struct CombatEndEvent {
  entt::entity winner; // Optional winner
  bool fled;
};

// Action completed event
struct ActionCompletedEvent {
  ActionID actionId;
  entt::entity entity;
};

} // namespace GameEvents
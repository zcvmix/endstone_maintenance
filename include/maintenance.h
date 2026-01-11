#pragma once

#include <endstone/plugin/plugin.hpp>
#include <endstone/event/player/player_join_event.h>
#include <endstone/scheduler/task.h>
#include <unordered_map>
#include <string>

class MaintenancePlugin : public endstone::Plugin {
public:
    void onEnable() override;
    void onDisable() override;
    void onPlayerJoin(endstone::PlayerJoinEvent &event);

private:
    void sendLoginWindow(endstone::Player &player);

    std::unordered_map<endstone::UUID, endstone::Task *> pending_kicks_;
};

#include "maintenance.h"

#include <endstone/color_format.h>
#include <endstone/form/modal_form.h>
#include <endstone/form/controls/text_input.h>
#include <endstone/scheduler/scheduler.h>

ENDSTONE_PLUGIN("maintenance", "1.0.0", MaintenancePlugin)
{
    description = "Maintenance Plugin.";
    authors = {"zcvmix or sphereto"};
    permission("maintenance.bypass").description("Bypass maintenance mode").default_(endstone::PermissionDefault::Operator);
}

void MaintenancePlugin::onEnable()
{
    saveDefaultConfig();
    registerEvent(&MaintenancePlugin::onPlayerJoin, *this);
    getLogger().info("Maintenance Plugin enabled.");
}

void MaintenancePlugin::onDisable()
{
    getLogger().info("Maintenance Plugin disabled.");
}

void MaintenancePlugin::onPlayerJoin(endstone::PlayerJoinEvent &event)
{
    auto &config = getConfig();
    bool enabled = config.getBoolean("maintenance.enabled", false);
    if (!enabled) {
        return;
    }

    endstone::Player &player = event.getPlayer();

    if (player.hasPermission("maintenance.bypass")) {
        player.sendMessage(endstone::ColorFormat::Green + "Maintenance is active, but you have bypass permission.");
        return;
    }

    sendLoginWindow(player);
}

void MaintenancePlugin::sendLoginWindow(endstone::Player &player)
{
    auto &config = getConfig();
    int delay = config.getInt("maintenance.kick_delay", 15);
    std::string password = config.getString("maintenance.password", "passwordhere");

    endstone::UUID uuid = player.getUniqueId();
    auto task = getServer().getScheduler().runTaskLater(*this, [this, uuid]() {
        auto *player = getServer().getPlayer(uuid);
        if (player) {
            player->kick("You took too long to login during maintenance.");
        }
        pending_kicks_.erase(uuid);
    }, delay * 20);

    pending_kicks_[uuid] = task.get();

    auto form = std::make_unique<endstone::ModalForm>();
    form->setTitle("Server Maintenance");
    
    form->addControl(std::make_shared<endstone::Label>("The server is currently in maintenance mode."));
    form->addControl(std::make_shared<endstone::TextInput>("password_input", "Enter Password", "Password"));

    form->setSubmitButton("Login");

    form->setOnSubmit([this, uuid, password](endstone::Player &p, std::string json_response) {
        if (pending_kicks_.find(uuid) != pending_kicks_.end()) {
            pending_kicks_[uuid]->cancel();
            pending_kicks_.erase(uuid);
        }
        if (json_response.find(password) != std::string::npos) {
            p.sendMessage(endstone::ColorFormat::Green + "Password accepted. Welcome!");
            p.sendTitle("Welcome", "Maintenance Mode");
        } else {
            p.kick("Wrong Password!");
        }
    });

    form->setOnClose([this, uuid](endstone::Player &p) {
        if (pending_kicks_.find(uuid) != pending_kicks_.end()) {
            pending_kicks_[uuid]->cancel();
            pending_kicks_.erase(uuid);
        }
        p.kick("Come Check Back Soon!");
    });

    player.sendForm(std::move(form));
}

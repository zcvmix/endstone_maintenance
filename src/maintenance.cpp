#include "maintenance.h"

#include <endstone/color_format.h>
#include <endstone/form/modal_form.h>
#include <endstone/form/controls/label.h>
#include <endstone/form/controls/text_input.h>
#include <endstone/scheduler/scheduler.h>
#include <endstone/potion/potion_effect.h>
#include <endstone/potion/potion_effect_type.h>

ENDSTONE_PLUGIN("maintenance", "1.0.0", MaintenancePlugin)
{
    description = "Maintenance Plugin.";
    authors = {"zcvmix or sphereto"};
    permission("maintenance.bypass").description("Bypass maintenance mode").default_(endstone::PermissionDefault::Operator);
}

void MaintenancePlugin::onEnable()
{
    // Initialize Config
    config_ = std::make_unique<SimpleConfig>("plugins/maintenance", "config.yml");
    
    std::string default_config = 
        "maintenance.enabled: true\n"
        "maintenance.password: passwordhere\n"
        "maintenance.kick_delay: 15\n";
    
    config_->saveDefault(default_config);

    registerEvent(&MaintenancePlugin::onPlayerJoin, *this);
    getLogger().info("Maintenance Plugin enabled.");
}

void MaintenancePlugin::onDisable()
{
    getLogger().info("Maintenance Plugin disabled.");
}

void MaintenancePlugin::onPlayerJoin(endstone::PlayerJoinEvent &event)
{
    config_->reload();

    bool enabled = getConfig().getBoolean("maintenance.enabled", true);
    if (!enabled) {
        return;
    }

    endstone::Player &player = event.getPlayer();

    if (player.hasPermission("maintenance.bypass")) {
        player.sendMessage(endstone::ColorFormat::Green + "Maintenance is active, but you have bypass permission.");
        return;
    }

    player.addPotionEffect(endstone::PotionEffect(endstone::PotionEffectType::Blindness, 1000000, 255));

    sendLoginWindow(player);
}

void MaintenancePlugin::sendLoginWindow(endstone::Player &player)
{
    int delay = getConfig().getInt("maintenance.kick_delay", 15);
    std::string password = getConfig().getString("maintenance.password", "passwordhere");

    endstone::UUID uuid = player.getUniqueId();
    
    auto task = getServer().getScheduler().runTaskLater(*this, [this, uuid]() {
        auto *p = getServer().getPlayer(uuid);
        if (p) {
            p->kick("You took too long to login during maintenance.");
        }
        pending_kicks_.erase(uuid);
    }, delay * 20);

    pending_kicks_[uuid] = task.get();

    auto form = std::make_unique<endstone::ModalForm>();
    form->setTitle("Server Maintenance");
    
    std::string time_label = "§6Time Left: §a" + std::to_string(delay) + "§r";
    form->addControl(endstone::Label(time_label));
    form->addControl(endstone::Label("The server is currently in maintenance mode."));
    form->addControl(endstone::TextInput("Password To Enter The Server", "Enter Password", ". . ."));

    form->setSubmitButton("Login");

    form->setOnSubmit([this, uuid, password](endstone::Player *p, std::string json_response) {
        if (!p) return;

        if (pending_kicks_.find(uuid) != pending_kicks_.end()) {
            if (pending_kicks_[uuid]) pending_kicks_[uuid]->cancel();
            pending_kicks_.erase(uuid);
        }

        if (json_response.find(password) != std::string::npos)
            p->removePotionEffect(endstone::PotionEffectType::Blindness);
            p->sendMessage(endstone::ColorFormat::Green + "Password accepted. Welcome!");
            p->sendTitle("Welcome", "Maintenance Mode");
        } else {
            p->kick("Wrong Password!");
        }
    });

    form->setOnClose([this, uuid](endstone::Player *p) {
        if (!p) return;

        if (pending_kicks_.find(uuid) != pending_kicks_.end()) {
            if (pending_kicks_[uuid]) pending_kicks_[uuid]->cancel();
            pending_kicks_.erase(uuid);
        }
        p->kick("Come Check Back Soon!");
    });

    player.sendForm(*form);
}

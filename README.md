# Maintenance Plugin

A lightweight C++ plugin for Endstone servers that enforces a maintenance mode with password protection.

## Function
When enabled, this plugin restricts server access by requiring players to enter a password upon joining.
- **Login System:** Displays a form requiring a password.
- **Auto-Kick:** Automatically kicks players who fail to enter the correct password within the configured time limit (default: 15 seconds).
- **Visual Effects:** Applies blindness to players until they successfully log in.
- **Bypass:** Operators and players with the `maintenance.bypass` permission can skip the login process.

## Version
Current Version: **1.0.0**

## License
This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

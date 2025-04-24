#pragma once

class NamingDataManager {
public:
    NamingDataManager() = default;

    ThreadSafe<std::unordered_set<std::string>> &active_servers() { return active_servers_; }
    ThreadSafe<std::unordered_map<std::string, std::unordered_set<std::string>>>  &files() { return files_; }
    ThreadSafe<std::unordered_map<std::string, std::chrono::steady_clock::time_point>> &server_state() {
        return server_state_;
    }

    const ThreadSafe<std::unordered_set<std::string>> &active_servers() const { return active_servers_; }
    const ThreadSafe<std::unordered_map<std::string, std::unordered_set<std::string>>>  &files() const { return files_; }
    const ThreadSafe<std::unordered_map<std::string, std::chrono::steady_clock::time_point>> &server_state() const {
        return server_state_;
    }

    void updateHeartbeat(const std::string &address) {
        server_state_.write([&](auto &m) { m[address] = std::chrono::steady_clock::now(); });
    }

    void addServerForFile(const std::string &filepath, const std::string &address) {
        files_.write([&](auto &s) { s[filepath].insert(address); });
    }

    void removeServerForFile(const std::string &filepath, const std::string &address) {
        files_.write([&](auto &s) { s[filepath].erase(address); });
    }

    void log() const {
        std::ostringstream oss;

        // Log active servers
        oss << "\nActive Servers:\n";
        active_servers_.read([&](const auto &servers) {
            for (const auto &server : servers) {
                oss << "  - " << server << "\n";
            }
        });

        // Log files
        oss << "Files:\n";
        files_.read([&](const auto &file_set) {
            for (const auto &file : file_set | std::views::keys) {
                oss << "  - " << file << "\n";
            }
        });

        // Log server state
        oss << "Server State:\n";
        server_state_.read([&](const auto &state) {
            for (const auto &[server, _] : state) {
                oss << "  - " << server << "\n";
            }
        });

        spdlog::info(oss.str());
    }

private:
    ThreadSafe<std::unordered_set<std::string>> active_servers_;
    ThreadSafe<std::unordered_map<std::string, std::unordered_set<std::string>>> files_;
    ThreadSafe<std::unordered_map<std::string, std::chrono::steady_clock::time_point>> server_state_;
};

#pragma once

class DataManager {
public:
    DataManager() = default;

    ThreadSafe<std::unordered_set<std::string>> &active_servers() { return active_servers_; }
    ThreadSafe<std::unordered_set<std::string>> &files() { return files_; }
    ThreadSafe<std::unordered_map<std::string, std::chrono::steady_clock::time_point>> &server_state() {
        return server_state_;
    }

    const ThreadSafe<std::unordered_set<std::string>> &active_servers() const { return active_servers_; }
    const ThreadSafe<std::unordered_set<std::string>> &files() const { return files_; }
    const ThreadSafe<std::unordered_map<std::string, std::chrono::steady_clock::time_point>> &server_state() const {
        return server_state_;
    }

    void updateHeartbeat(const std::string &address) {
        server_state_.write([&](auto &m) { m[address] = std::chrono::steady_clock::now(); });
    }

private:
    ThreadSafe<std::unordered_set<std::string>> active_servers_;
    ThreadSafe<std::unordered_set<std::string>> files_;
    ThreadSafe<std::unordered_map<std::string, std::chrono::steady_clock::time_point>> server_state_;
};

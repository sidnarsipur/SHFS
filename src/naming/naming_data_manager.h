#pragma once

struct Task {
    std::string source;
    std::string filepath;

    bool operator==(const Task& other) const {
        return source == other.source && filepath == other.filepath;
    }
};

struct Server {
    std::unordered_set<std::string> files;
    std::chrono::steady_clock::time_point last_heartbeat;
};

class NamingDataManager {
public:
    const int delay;
    const int replication_factor;

    explicit NamingDataManager(int delay = 0, int replication_factor = 2)
        : delay(delay), replication_factor(replication_factor) {}

    ThreadSafe<std::unordered_map<std::string, std::unordered_set<std::string>>>  &files() { return files_; }
    ThreadSafe<std::unordered_map<std::string, Server>> &servers() { return servers_; }
    ThreadSafe<std::unordered_map<std::string, std::vector<Task>>> &replication_tasks() { return replication_tasks_; }

    const ThreadSafe<std::unordered_map<std::string, std::unordered_set<std::string>>>  &files() const { return files_; }
    const ThreadSafe<std::unordered_map<std::string, Server>> &servers() const { return servers_; }
    const ThreadSafe<std::unordered_map<std::string, std::vector<Task>>> &replication_tasks() const {
        return replication_tasks_;
    }

    void newServer(const std::string &address) {
        servers_.write([&](auto &m) {
            m[address] = Server{{}, std::chrono::steady_clock::now()};
        });
    }

    void updateHeartbeat(const std::string &address) {
        servers_.write([&](auto &m) {
            if (m.contains(address)) {
                m[address].last_heartbeat = std::chrono::steady_clock::now();
            }
        });
    }

    void addServerForFile(const std::string &filepath, const std::string &address) {
        files_.write([&](auto &s) { s[filepath].insert(address); });
    }

    void removeServerForFile(const std::string &filepath, const std::string &address) {
        files_.write([&](auto &s) { s[filepath].erase(address); });
    }

    std::vector<std::string> getLeastLoadedServers(int n) {
        return servers_.read([&](const auto &m) {
            std::vector<std::pair<std::string, int>> server_loads;
            for (const auto &[address, server] : m) {
                server_loads.emplace_back(address, server.files.size());
            }
            std::sort(server_loads.begin(), server_loads.end(),
                      [](const auto &a, const auto &b) { return a.second < b.second; });
            std::vector<std::string> least_loaded_servers;
            for (int i = 0; i < n && i < server_loads.size(); ++i) {
                least_loaded_servers.push_back(server_loads[i].first);
            }
            return least_loaded_servers;
        });
    }

    bool fileExists(const std::string &filepath) const {
        return files().read([&](const auto &files_map) {
            return files_map.contains(filepath);
        });
    }

    std::vector<Task> getReplicationTasks(const std::string &replica_address) {
         return replication_tasks_.write([&](auto &m) {
            if (m.contains(replica_address)) {
                std::vector<Task> tasks = m.at(replica_address);
                m.erase(replica_address);
                return tasks;
            }
            return std::vector<Task>{};
        });
    }

    void log() const {
        std::ostringstream oss;

        // Log files
        oss << "Files:\n";
        files_.read([&](const auto &file_set) {
            for (const auto &file : file_set | std::views::keys) {
                oss << "  - " << file << "\n";
            }
        });

        // Log replication tasks
        oss << "Replication Tasks:\n";
        replication_tasks_.read([&](const auto &tasks) {
            for (const auto &[file, task_list] : tasks) {
                oss << "  - " << file << "\n";
                for (const auto &task : task_list) {
                    oss << "    - " << task.source << " -> " << task.filepath << "\n";
                }
            }
        });

        spdlog::info(oss.str());
    }

private:
    ThreadSafe<std::unordered_map<std::string, std::unordered_set<std::string>>> files_;
    ThreadSafe<std::unordered_map<std::string, Server>> servers_;
    ThreadSafe<std::unordered_map<std::string, std::vector<Task>>> replication_tasks_;
};

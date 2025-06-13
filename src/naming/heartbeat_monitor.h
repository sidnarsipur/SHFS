#pragma once

class HeartbeatMonitor {
public:
    explicit HeartbeatMonitor(std::shared_ptr<NamingDataManager> manager, const int interval = 5, const int timeout = 10)
        : dm(std::move(manager)), interval_(interval), timeout_(timeout), stop_flag_(false) {}

    void Start() {
        thread_ = std::thread([this] {
            while (!stop_flag_.load()) {
                auto servers = dm->servers().get();
                std::vector<std::string> down_servers;

                // Generate a list of down servers
                for (const auto &[addr, server]: servers) {
                    if (std::chrono::steady_clock::now() - server.last_heartbeat > std::chrono::seconds(timeout_)) {
                        down_servers.push_back(addr);
                    }
                }
                if (down_servers.empty()) {
                    std::this_thread::sleep_for(std::chrono::seconds(interval_));
                    continue;
                }

                // Remove down servers from active servers
                for (const auto &addr: down_servers) {
                    spdlog::critical("Server {} is down", addr);
                    servers.erase(addr);
                }
                dm->servers().set(servers);

                if (servers.size() < dm->replication_factor) {
                    spdlog::critical("Not enough active servers for replication! Please bring up more servers");
                    std::this_thread::sleep_for(std::chrono::seconds(interval_));
                    continue;
                }

                std::unordered_set<std::string> active_servers;
                for (const auto &[addr, server]: servers) {
                    active_servers.insert(addr);
                }

                // Remove down servers from files, and generate a list of replication tasks
                std::unordered_map<std::string, std::vector<Task>> replication_tasks;
                dm->files().write([&](auto &s) {
                    for (auto &[file, serversWithFile]: s) {
                        for (const auto &addr: down_servers) {
                            serversWithFile.erase(addr);
                        }
                        if (serversWithFile.empty()) {
                            s.erase(file);
                            spdlog::critical("File {} is not available anymore", file);
                            continue;
                        }
                        if (serversWithFile.size() < dm->replication_factor) {
                            spdlog::critical("File {} is underreplicated! replicating it to active servers...", file);
                            addReplicationTask(serversWithFile, active_servers, replication_tasks, file);
                        }
                    }
                });

                dm->replication_tasks().set(replication_tasks);
                std::this_thread::sleep_for(std::chrono::seconds(interval_));
            }
        });
    }

    void Stop() {
        stop_flag_.store(true);
        if (thread_.joinable()) {
            thread_.join();
        }
    }


    void test_basic_single_source() {
        std::unordered_set<std::string> sources = {"A"};
        std::unordered_set<std::string> active_servers = {"A", "B", "C", "D"};
        std::unordered_map<std::string, std::vector<Task>> replication_tasks;
        std::string filepath = "/file1.txt";

        addReplicationTask(sources, active_servers, replication_tasks, filepath);

        assert(replication_tasks.size() == 2); // need 2 replicas

        assert(replication_tasks["B"][0].source == "A");
        assert(replication_tasks["B"][0].filepath == filepath);
        assert(replication_tasks["C"][0].source == "A");
        assert(replication_tasks["C"][0].filepath == filepath);
    }

    void test_multiple_sources() {
        std::unordered_set<std::string> sources = {"A", "B"};
        std::unordered_set<std::string> active_servers = {"A", "B", "C", "D"};
        std::unordered_map<std::string, std::vector<Task>> replication_tasks;
        std::string filepath = "/file2.txt";

        addReplicationTask(sources, active_servers, replication_tasks, filepath);

        assert(replication_tasks.size() == 1); // 1 replica needed

        std::string replica = replication_tasks.begin()->first;
        Task task = replication_tasks[replica][0];
        assert((task.source == "A" || task.source == "B") && task.filepath == filepath);

        std::cout << "✅ test_multiple_sources passed\n";
    }

private:
    std::shared_ptr<NamingDataManager> dm;
    int interval_;
    int timeout_;
    std::atomic<bool> stop_flag_;
    std::thread thread_;

    void addReplicationTask(
        const std::unordered_set<std::string> &sources,
        const std::unordered_set<std::string> &active_servers,
        std::unordered_map<std::string, std::vector<Task>> &replication_tasks,
        const std::string &filepath
    ) const {
        // Step 1: Build list of potential replicas (active but not sources)
        std::vector<std::string> replicas;
        for (const auto &val: active_servers) {
            if (!sources.contains(val)) {
                replicas.push_back(val);
            }
        }

        // Step 2: Take the first k replicas
        unsigned int k = dm->replication_factor - sources.size();
        if (k > replicas.size()) k = replicas.size();
        const std::vector selected_replicas(replicas.begin(), replicas.begin() + k);

        // Convert sources to vector for indexing
        const std::vector source_list(sources.begin(), sources.end());

        // Step 3: Build replica → source map (round-robin or sequential assignment)
        for (size_t i = 0; i < selected_replicas.size(); ++i) {
            const std::string &replica = selected_replicas[i];
            const std::string &source = source_list[i % source_list.size()]; // round-robin
            replication_tasks[replica].push_back({source, filepath});
        }
    }
};


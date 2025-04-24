#pragma once

class HeartbeatMonitor {
public:
    explicit HeartbeatMonitor(std::shared_ptr<NamingDataManager> manager, const int interval, const int timeout)
        : dm(std::move(manager)), interval_(interval), timeout_(timeout), stop_flag_(false) {}

    void Start() {
        thread_ = std::thread([this] {
            while (!stop_flag_.load()) {
                std::vector<std::string> down_servers;
                dm->server_state().read([&](const auto &m) {
                    for (const auto &[addr, time]: m) {
                        if (std::chrono::steady_clock::now() - time > std::chrono::seconds(timeout_)) {
                            down_servers.push_back(addr);
                        }
                    }
                });

                for (const auto &addr: down_servers) {
                    spdlog::info("Server {} is down", addr);
                    dm->active_servers().write([&](auto &s) { s.erase(addr); });
                }
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

private:
    std::shared_ptr<NamingDataManager> dm;
    int interval_;
    int timeout_;
    std::atomic<bool> stop_flag_;
    std::thread thread_;
};

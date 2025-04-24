#pragma once

class StorageDataManager {
public:
    StorageDataManager() = default;

    ThreadSafe<std::unordered_set<std::string>> &files() { return files_; }

    const ThreadSafe<std::unordered_set<std::string>> &files() const { return files_; }

    void addFile(const std::string &filepath) {
        files_.write([&](auto &set) { set.insert(filepath); });
    }

    void removeFile(const std::string &filepath) {
        files_.write([&](auto &set) { set.erase(filepath); });
    }

    bool fileExists(const std::string &filepath) {
        return files_.get().find(filepath) != files_.get().end();
    }

private:
    ThreadSafe<std::unordered_set<std::string>> files_;
};
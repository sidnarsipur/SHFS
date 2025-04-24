#pragma once

class StorageDataManager {
public:
    StorageDataManager() = default;

    ThreadSafe<std::unordered_set<std::string>> &files() { return files_; }

    const ThreadSafe<std::unordered_set<std::string>> &files() const { return files_; }

private:
    ThreadSafe<std::unordered_set<std::string>> files_;
};
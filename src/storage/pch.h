#pragma once
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <memory>
#include <spdlog/spdlog.h>
#include "naming.grpc.pb.h"
#include "storage.grpc.pb.h"
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include <iostream>
#include <fstream>
#include "../naming/thread_safe.h"
#include <filesystem>
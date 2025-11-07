#include "file_manager.h"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json.hpp>
#include <random>
#include <sstream>

using json = nlohmann::json;

#define METADATA_FILE "meta/file_metadata.json"

// 生成随机删除码（8位字母数字组合）
std::string generate_delete_code() {
  static const char charset[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

  std::string code;
  code.reserve(8);
  for (int i = 0; i < 8; ++i) {
    code += charset[dis(gen)];
  }
  return code;
}

// 获取当前时间的 ISO 8601 格式字符串
std::string get_current_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%dT%H:%M:%S");
  return ss.str();
}

// 保存文件元数据（使用 nlohmann/json）
bool save_file_metadata(const std::string &filename, size_t size,
                        const std::string &timestamp,
                        const std::string &delete_code) {
  // 确保 meta 目录存在
  std::filesystem::path meta_dir("meta");
  if (!std::filesystem::exists(meta_dir)) {
    std::filesystem::create_directory(meta_dir);
  }

  json metadata_array;

  // 读取现有元数据
  std::ifstream ifs(METADATA_FILE);
  if (ifs.is_open()) {
    try {
      ifs >> metadata_array;
      ifs.close();
    } catch (const json::exception &e) {
      ifs.close();
      metadata_array = json::array(); // 解析失败，创建新数组
    }
  } else {
    metadata_array = json::array(); // 文件不存在，创建新数组
  }

  // 如果不是数组，重新创建
  if (!metadata_array.is_array()) {
    metadata_array = json::array();
  }

  // 创建新的文件条目
  json item = {{"filename", filename},
               {"size", size},
               {"uploadTime", timestamp},
               {"code", delete_code}};

  // 添加到数组
  metadata_array.push_back(item);

  // 写入文件
  std::ofstream ofs(METADATA_FILE);
  if (!ofs.is_open()) {
    return false;
  }

  ofs << metadata_array.dump(2); // 格式化输出，缩进2个空格
  ofs.close();

  return true;
}

// 根据删除码删除文件及其元数据（使用 nlohmann/json）
bool delete_file_by_code(const std::string &delete_code,
                         std::string &deleted_filename) {
  // 读取元数据
  std::ifstream ifs(METADATA_FILE);
  if (!ifs.is_open()) {
    return false;
  }

  json metadata_array;
  try {
    ifs >> metadata_array;
    ifs.close();
  } catch (const json::exception &e) {
    ifs.close();
    return false;
  }

  if (!metadata_array.is_array()) {
    return false;
  }

  // 查找并删除匹配的条目
  bool found = false;
  for (auto it = metadata_array.begin(); it != metadata_array.end(); ++it) {
    if (it->contains("code") && (*it)["code"] == delete_code) {
      // 找到了，提取文件名
      if (it->contains("filename")) {
        deleted_filename = (*it)["filename"].get<std::string>();
      }

      // 从数组中删除
      metadata_array.erase(it);
      found = true;
      break;
    }
  }

  if (!found) {
    return false;
  }

  // 删除实际文件
  std::filesystem::path filepath =
      std::filesystem::path("assets") / deleted_filename;
  if (std::filesystem::exists(filepath)) {
    try {
      std::filesystem::remove(filepath);
    } catch (const std::exception &e) {
      std::cerr << "Failed to delete file: " << e.what() << std::endl;
      return false;
    }
  }

  // 更新元数据文件
  std::ofstream ofs(METADATA_FILE);
  if (!ofs.is_open()) {
    return false;
  }

  ofs << metadata_array.dump(2); // 格式化输出
  ofs.close();

  return true;
}

// 读取所有文件元数据
std::string read_file_metadata() {
  std::ifstream ifs(METADATA_FILE);
  if (!ifs.is_open()) {
    // 如果文件不存在，返回空数组
    return "[]";
  }

  std::stringstream buffer;
  buffer << ifs.rdbuf();
  ifs.close();

  return buffer.str();
}

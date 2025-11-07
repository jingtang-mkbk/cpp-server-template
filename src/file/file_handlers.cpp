#include "file_handlers.h"
#include "file_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <string>

using json = nlohmann::json;

// 判断文件类型（image/video/other）
static std::string get_file_type(const std::string &ext) {
  // 图片格式
  if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" ||
      ext == ".bmp" || ext == ".webp" || ext == ".svg" || ext == ".ico") {
    return "image";
  }
  // 视频格式
  else if (ext == ".mp4" || ext == ".avi" || ext == ".mov" || ext == ".wmv" ||
           ext == ".flv" || ext == ".webm" || ext == ".mkv" || ext == ".m4v" ||
           ext == ".3gp" || ext == ".mpg" || ext == ".mpeg") {
    return "video";
  }
  // 其他格式
  return "other";
}

// 根据文件扩展名获取 Content-Type
static std::string get_content_type(const std::filesystem::path &filepath) {
  std::string ext = filepath.extension().string();

  if (ext == ".html" || ext == ".htm") {
    return "text/html; charset=utf-8";
  } else if (ext == ".css") {
    return "text/css; charset=utf-8";
  } else if (ext == ".js") {
    return "application/javascript; charset=utf-8";
  } else if (ext == ".json") {
    return "application/json; charset=utf-8";
  } else if (ext == ".png") {
    return "image/png";
  } else if (ext == ".jpg" || ext == ".jpeg") {
    return "image/jpeg";
  } else if (ext == ".gif") {
    return "image/gif";
  } else if (ext == ".svg") {
    return "image/svg+xml";
  } else if (ext == ".txt") {
    return "text/plain; charset=utf-8";
  } else if (ext == ".mp4") {
    return "video/mp4";
  } else if (ext == ".webm") {
    return "video/webm";
  } else if (ext == ".avi") {
    return "video/x-msvideo";
  } else if (ext == ".mov") {
    return "video/quicktime";
  }

  return "application/octet-stream";
}

// 验证文件名是否安全（防止路径遍历攻击）
static bool is_valid_filename(const std::string &filename) {
  return filename.find("..") == std::string::npos &&
         filename.find("/") == std::string::npos &&
         filename.find("\\") == std::string::npos;
}

// 处理 /api/file-upload 请求（文件上传）
void handle_file_upload(const httplib::Request &req, httplib::Response &res) {
  try {
    // 检查是否有文件被上传
    if (!req.form.has_file("file")) {
      res.status = 400;
      res.set_content(
          "{\"error\":\"No file uploaded. Use 'file' as the field name.\"}",
          "application/json; charset=utf-8");
      return;
    }

    const auto &file = req.form.get_file("file");

    // 获取文件名（优先使用客户端指定的文件名）
    std::string filename = file.filename;
    if (filename.empty()) {
      res.status = 400;
      res.set_content("{\"error\":\"Invalid filename\"}",
                      "application/json; charset=utf-8");
      return;
    }

    // 验证文件名安全性
    if (!is_valid_filename(filename)) {
      res.status = 400;
      res.set_content("{\"error\":\"Invalid filename. Filename cannot contain "
                      "path separators.\"}",
                      "application/json; charset=utf-8");
      return;
    }

    // 检查文件大小
    if (file.content.size() > MAX_FILE_SIZE) {
      res.status = 413;
      json error = {{"error", "File too large. Maximum size is 2GB."},
                    {"maxSize", MAX_FILE_SIZE},
                    {"fileSize", file.content.size()}};
      res.set_content(error.dump(), "application/json; charset=utf-8");
      return;
    }

    // 确保 assets 目录存在
    std::filesystem::path assets_dir("assets");
    if (!std::filesystem::exists(assets_dir)) {
      std::filesystem::create_directory(assets_dir);
    }

    // 构建目标文件路径
    std::filesystem::path filepath = assets_dir / filename;

    // 检查文件是否已存在（可选：如果需要覆盖，移除此检查）
    if (std::filesystem::exists(filepath)) {
      res.status = 409;
      res.set_content("{\"error\":\"File already exists\"}",
                      "application/json; charset=utf-8");
      return;
    }

    // 写入文件
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs.is_open()) {
      res.status = 500;
      res.set_content("{\"error\":\"Failed to save file\"}",
                      "application/json; charset=utf-8");
      return;
    }

    ofs.write(file.content.data(), file.content.size());
    ofs.close();

    // 保存文件元数据
    std::string timestamp = get_current_timestamp();
    std::string delete_code = generate_delete_code();
    if (!save_file_metadata(filename, file.content.size(), timestamp,
                            delete_code)) {
      std::cerr << "Warning: Failed to save file metadata for " << filename
                << std::endl;
    }

    // 返回成功响应（使用 nlohmann/json）
    json response = {{"success", true},
                     {"filename", filename},
                     {"size", file.content.size()},
                     {"uploadTime", timestamp},
                     {"code", delete_code},
                     {"path", "/api/file-get?name=" + filename}};

    res.set_content(response.dump(), "application/json; charset=utf-8");
  } catch (const std::exception &e) {
    // 捕获所有异常
    res.status = 500;
    std::cerr << "Exception in file upload: " << e.what() << std::endl;
    json error = {{"error", "Internal server error"}, {"details", e.what()}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
  } catch (...) {
    // 捕获未知异常
    res.status = 500;
    std::cerr << "Unknown exception in file upload" << std::endl;
    res.set_content("{\"error\":\"Unknown internal error\"}",
                    "application/json; charset=utf-8");
  }
}

// 处理 /api/file-get 请求（文件下载）
void handle_file_get(const httplib::Request &req, httplib::Response &res) {
  // 获取查询参数 name
  std::string filename = req.get_param_value("name");
  if (filename.empty()) {
    res.status = 400;
    res.set_content("{\"error\":\"Missing parameter 'name'\"}",
                    "application/json; charset=utf-8");
    return;
  }

  // 防止路径遍历攻击，只允许文件名，不允许路径分隔符
  if (!is_valid_filename(filename)) {
    res.status = 400;
    res.set_content("{\"error\":\"Invalid filename\"}",
                    "application/json; charset=utf-8");
    return;
  }

  // 构建文件路径
  std::filesystem::path filepath = std::filesystem::path("assets") / filename;

  // 检查文件是否存在
  if (!std::filesystem::exists(filepath) ||
      !std::filesystem::is_regular_file(filepath)) {
    res.status = 404;
    res.set_content("{\"error\":\"File not found\"}",
                    "application/json; charset=utf-8");
    return;
  }

  // 读取文件内容
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    res.status = 500;
    res.set_content("{\"error\":\"Failed to open file\"}",
                    "application/json; charset=utf-8");
    return;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  // 根据文件扩展名设置 Content-Type
  std::string content_type = get_content_type(filepath);
  res.set_content(content, content_type);
}

// 处理 /api/file-list 请求（获取所有上传文件的信息）
void handle_file_list([[maybe_unused]] const httplib::Request &req,
                      httplib::Response &res) {
  std::string metadata_str = read_file_metadata();

  // 解析元数据
  json file_list;
  try {
    file_list = json::parse(metadata_str);
  } catch (const json::exception &e) {
    file_list = json::array();
  }

  // 构建标准响应格式
  json response = {{"success", true}, {"data", file_list}};

  res.set_content(response.dump(2), "application/json; charset=utf-8");
}

// 处理 /api/file-delete 请求（通过删除码删除文件）
void handle_file_delete_by_code(const httplib::Request &req,
                                httplib::Response &res) {
  // 获取查询参数 code
  std::string delete_code = req.get_param_value("code");
  if (delete_code.empty()) {
    res.status = 400;
    res.set_content("{\"error\":\"Missing parameter 'code'\"}",
                    "application/json; charset=utf-8");
    return;
  }

  // 验证 code 格式（8位字母数字）
  if (delete_code.length() != 8) {
    res.status = 400;
    res.set_content("{\"error\":\"Invalid code format\"}",
                    "application/json; charset=utf-8");
    return;
  }

  // 删除文件
  std::string deleted_filename;
  if (delete_file_by_code(delete_code, deleted_filename)) {
    json response = {{"success", true}, {"filename", deleted_filename}};
    res.set_content(response.dump(), "application/json; charset=utf-8");
  } else {
    res.status = 404;
    json error = {{"error", "File not found or code invalid"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
  }
}

// 处理 /api/file-preview 请求（获取文件预览信息）
void handle_file_preview(const httplib::Request &req, httplib::Response &res) {
  // 获取查询参数 code
  std::string code = req.get_param_value("code");
  if (code.empty()) {
    res.status = 400;
    json error = {{"error", "Missing parameter 'code'"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
    return;
  }

  // 验证 code 格式（8位字母数字）
  if (code.length() != 8) {
    res.status = 400;
    json error = {{"error", "Invalid code format"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
    return;
  }

  // 从元数据中查找对应的文件
  std::ifstream ifs("meta/file_metadata.json");
  if (!ifs.is_open()) {
    res.status = 404;
    json error = {{"error", "Metadata not found"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
    return;
  }

  json metadata_array;
  try {
    ifs >> metadata_array;
    ifs.close();
  } catch (const json::exception &e) {
    ifs.close();
    res.status = 500;
    json error = {{"error", "Failed to parse metadata"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
    return;
  }

  // 查找匹配的文件
  std::string filename;
  bool found = false;

  for (const auto &item : metadata_array) {
    if (item.contains("code") && item["code"] == code) {
      if (item.contains("filename")) {
        filename = item["filename"].get<std::string>();
        found = true;
        break;
      }
    }
  }

  if (!found) {
    res.status = 404;
    json error = {{"error", "File not found or code invalid"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
    return;
  }

  // 构建文件路径并检查是否存在
  std::filesystem::path filepath = std::filesystem::path("assets") / filename;
  if (!std::filesystem::exists(filepath) ||
      !std::filesystem::is_regular_file(filepath)) {
    res.status = 404;
    json error = {{"error", "File not found on disk"}};
    res.set_content(error.dump(), "application/json; charset=utf-8");
    return;
  }

  // 获取文件信息
  std::string ext = filepath.extension().string();
  std::string file_type = get_file_type(ext);
  size_t file_size = std::filesystem::file_size(filepath);

  // 构建响应
  json response = {
      {"filename", filename}, {"type", file_type}, {"size", file_size}};

  // 根据类型决定是否提供预览 URL
  if (file_type == "image" || file_type == "video") {
    response["url"] = "/api/file-get?name=" + filename;
  } else {
    response["url"] = "";
  }

  res.set_content(response.dump(2), "application/json; charset=utf-8");
}

#include <filesystem>
#include <fstream>
#include <httplib.h>
#include <iostream>
#include <string>

#define PORT 8080

// 根据文件扩展名获取 Content-Type
std::string get_content_type(const std::filesystem::path &filepath) {
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
  }

  return "application/octet-stream";
}

// 验证文件名是否安全（防止路径遍历攻击）
bool is_valid_filename(const std::string &filename) {
  return filename.find("..") == std::string::npos &&
         filename.find("/") == std::string::npos &&
         filename.find("\\") == std::string::npos;
}

// 处理 /api/test 请求
void handle_test([[maybe_unused]] const httplib::Request &req,
                 httplib::Response &res) {
  const std::string body = "{\"data\":\"success\"}";
  res.set_content(body, "application/json; charset=utf-8");
}

// 处理 /api/file 请求
void handle_file(const httplib::Request &req, httplib::Response &res) {
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  httplib::Server server;

  server.Get("/api/test", handle_test);
  server.Get("/api/file", handle_file);

  std::cout << "HTTP server listening on http://0.0.0.0:" << PORT << std::endl;
  server.listen("0.0.0.0", PORT);
  return 0;
}

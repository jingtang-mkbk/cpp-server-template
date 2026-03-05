#include "test_routes.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <httplib.h>
#include <json.hpp>
#include <memory>
#include <string>
#include <thread>

using json = nlohmann::json;

void handle_test([[maybe_unused]] const httplib::Request &req,
                 httplib::Response &res) {
  const std::string body = "{\"data\":\"success test\"}";
  res.set_content(body, "application/json; charset=utf-8");
}

struct StreamState {
  std::string merged;
  size_t pos;
  int phase; // 0=起始 1=数据 2=结束
  size_t chunk_index;
  static constexpr size_t CHUNK_SIZE = 100;
};

// 流式返回：合并 index.html、main.js、style.css，用
// ===index.html===、===index.js===、===index.css=== 分割
void handle_test_stream([[maybe_unused]] const httplib::Request &req,
                        httplib::Response &res) {
  std::string html_content, js_content, css_content;

  auto read_file = [&res](const std::string &path, std::string &out) -> bool {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
      res.status = 500;
      res.set_content(json{{"error", "Failed to open file: " + path}}.dump(),
                      "application/json; charset=utf-8");
      return false;
    }
    out.assign((std::istreambuf_iterator<char>(ifs)),
               std::istreambuf_iterator<char>());
    return true;
  };

  if (!read_file("test/index.html", html_content) ||
      !read_file("test/main.js", js_content) ||
      !read_file("test/style.css", css_content)) {
    return;
  }

  // 合并：用分隔符串联
  std::string merged = "===index.html===\n" + html_content +
                       "\n===index.js===\n" + js_content +
                       "\n===index.css===\n" + css_content;

  auto state = std::make_shared<StreamState>();
  state->merged = std::move(merged);
  state->pos = 0;
  state->phase = 0;
  state->chunk_index = 0;

  res.set_chunked_content_provider(
      "text/event-stream; charset=utf-8",
      [state]([[maybe_unused]] size_t offset, httplib::DataSink &sink) {
        // 阶段 0：发送起始
        if (state->phase == 0) {
          json start = {{"type", "start"},
                        {"total", state->merged.size()},
                        {"chunk_size", StreamState::CHUNK_SIZE}};
          std::string line = "data: " + start.dump() + "\n\n";
          if (!sink.write(line.data(), line.size()))
            return false;
          state->phase = 1;
          return true;
        }

        // 阶段 1：发送数据块
        if (state->phase == 1) {
          if (state->pos >= state->merged.size()) {
            state->phase = 2;
            return true;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          size_t chunk = (std::min)(StreamState::CHUNK_SIZE,
                                    state->merged.size() - state->pos);
          std::string chunk_str;
          while (chunk > 0) {
            chunk_str = state->merged.substr(state->pos, chunk);
            try {
              (void)json(chunk_str).dump();
              break;
            } catch (const json::exception &) {
              chunk--;
            }
          }
          if (chunk == 0)
            return false;
          json data = {{"type", "chunk"},
                       {"index", state->chunk_index},
                       {"content", chunk_str}};
          std::string line = "data: " + data.dump() + "\n\n";
          if (!sink.write(line.data(), line.size()))
            return false;
          state->pos += chunk;
          state->chunk_index++;
          return true;
        }

        // 阶段 2：发送结束
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        json end = {{"type", "end"},
                    {"total_chunks", state->chunk_index},
                    {"total_bytes", state->pos}};
        std::string line = "data: " + end.dump() + "\n\n";
        if (!sink.write(line.data(), line.size()))
          return false;
        return false;
      },
      nullptr);
}

// 配置测试相关路由
void configure_test_routes(httplib::Server &server) {
  server.Get("/api/test", handle_test);
  server.Get("/api/test-stream", handle_test_stream);
}

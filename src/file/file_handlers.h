#ifndef FILE_HANDLERS_H
#define FILE_HANDLERS_H

#include <httplib.h>

// 配置常量
#define MAX_FILE_SIZE (100 * 1024 * 1024) // 100MB 文件大小限制

// 文件操作处理函数

// 处理 /api/file-upload 请求（文件上传）
void handle_file_upload(const httplib::Request &req, httplib::Response &res);

// 处理 /api/file-get 请求（文件下载）
void handle_file_get(const httplib::Request &req, httplib::Response &res);

// 处理 /api/file-list 请求（获取所有上传文件的信息）
void handle_file_list(const httplib::Request &req, httplib::Response &res);

// 处理 /api/file-delete 请求（通过删除码删除文件）
void handle_file_delete_by_code(const httplib::Request &req,
                                httplib::Response &res);

// 处理 /api/file-preview 请求（获取文件预览信息）
void handle_file_preview(const httplib::Request &req, httplib::Response &res);

#endif // FILE_HANDLERS_H

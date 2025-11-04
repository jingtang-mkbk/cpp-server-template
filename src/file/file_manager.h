#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

// 生成随机删除码（8位字母数字组合）
std::string generate_delete_code();

// 获取当前时间的 ISO 8601 格式字符串
std::string get_current_timestamp();

// 保存文件元数据（使用 cJSON）
bool save_file_metadata(const std::string &filename, size_t size,
                        const std::string &timestamp,
                        const std::string &delete_code);

// 根据删除码删除文件及其元数据（使用 cJSON）
bool delete_file_by_code(const std::string &delete_code,
                         std::string &deleted_filename);

// 读取所有文件元数据
std::string read_file_metadata();

#endif // FILE_MANAGER_H

#pragma once

constexpr auto filename(std::string_view file) -> std::string_view {
  return file.substr(file.find_last_of("/") + 1, file.size() - 1);
}

constexpr auto func_sig(std::string_view fn) -> std::string_view {
  auto finish = fn.find_first_of("(");
  auto start = fn.find_last_of(" ", finish);
  return std::string_view(fn.data() + start + 1, finish - start - 1);
}

#define log_info(FORMAT, ...) fmt::println("\033[32m[INFO]\033[0m: {} (file: {}, line: {}, from: {})", fmt::format(FORMAT, __VA_ARGS__), filename(__FILE__), __LINE__, func_sig(std::source_location::current().function_name()))
#define log_warn(FORMAT, ...) fmt::println("\033[33m[WARN]\033[0m: {} (file: {}, line: {}, from: {})", fmt::format(FORMAT, __VA_ARGS__), filename(__FILE__), __LINE__, func_sig(std::source_location::current().function_name()))
#define log_error(FORMAT, ...) fmt::println("\033[31m[ERROR]\033[0m: {} (file: {}, line: {}, from: {})", fmt::format(FORMAT, __VA_ARGS__), filename(__FILE__), __LINE__, func_sig(std::source_location::current().function_name()))
#define log_fatal(FORMAT, ...) fmt::println("\033[31m[FATAL]\033[0m: {} (file: {}, line: {}, from: {})", fmt::format(FORMAT, __VA_ARGS__), filename(__FILE__), __LINE__, func_sig(std::source_location::current().function_name()))
#define ASSERT(condition, message, ...) if(!(condition)) { fmt::println("\033[31mASSERTION FAILED\033[0m: {}, {} (file: {}, line: {}, from: {})", #condition, fmt::format(message, __VA_ARGS__), filename(__FILE__), __LINE__, func_sig(std::source_location::current().function_name())); __debugbreak(); }
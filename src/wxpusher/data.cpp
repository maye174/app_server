
#include "wxpusher/inc/data.hpp"

#include <iostream>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

using json = nlohmann::json;

void api_user_attention_callback(struct evhttp_request *req, void *arg) {
    // 获取请求正文
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t input_len = evbuffer_get_length(input_buffer);
    char *input_data = new char[input_len];
    evbuffer_remove(input_buffer, input_data, input_len);

    // 解析JSON
    json j = json::parse(input_data);
    int appId = j["data"]["appId"].get<int>();
    std::string uid = j["data"]["uid"].get<std::string>();
    std::string extra = j["data"]["extra"].get<std::string>();

    // 打开数据库
    sqlite3 *db;
    int rc = sqlite3_open("wxpusher_app_data.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // 创建表（如果不存在）
    const char *sql_create_table =
        "CREATE TABLE IF NOT EXISTS wxpusher_app_data ("
        "appId INTEGER,"
        "uid TEXT,"
        "extra TEXT);";
    char *err_msg = nullptr;
    rc = sqlite3_exec(db, sql_create_table, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    // 插入数据
    std::string sql_insert =
        "INSERT INTO wxpusher_app_data (appId, uid, extra) VALUES (" +
        std::to_string(appId) + ", '" + uid + "', '" + extra + "');";
    rc = sqlite3_exec(db, sql_insert.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    // 关闭数据库
    sqlite3_close(db);

    delete[] input_data;
}

using list = std::vector<std::tuple<std::string, std::string>>;

list query_data_by_appid(int appid) {
    sqlite3 *db;
    list results;

    int rc = sqlite3_open("app_data.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return results;
    }

    std::string sql_query = "SELECT uid, extra FROM app_data WHERE appId = " +
                            std::to_string(appid) + ";";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string uid =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string extra =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            results.emplace_back(uid, extra);
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return results;
}

#include "wxpusher/inc/data.hpp"

#include <mutex>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <loguru.hpp>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

using json = nlohmann::json;

static std::mutex mtx;

void api_user_attention_callback(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        LOG_F(ERROR,
              "ewarn_api_create_qrcode: Failed to create response buffer");
        evhttp_send_error(req, 500, "Internal Server Error");
        return;
    }

    auto input_data = (char *)arg;
    LOG_F(INFO, "%s", input_data);

    // 解析JSON
    json j;
    try {
        j = json::parse(input_data);
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "解析JSON数据时发生异常 - %s", e.what());
    } catch (const json::type_error &e) {
        LOG_F(ERROR, "JSON数据类型错误 - %s", e.what());
    } catch (const std::exception &e) {
        LOG_F(ERROR, "发生未知异常 - %s", e.what());
    }

    if (!j.contains("data") || !j["data"].contains("appId") ||
        !j["data"].contains("uid") || !j["data"].contains("extra")) {
        LOG_F(ERROR, "未找到data.appId 或者 data.uid 或者 data.extra\n 400");
        evhttp_send_error(req, 400, "Bad Request");
        evbuffer_free(buf);
        return;
    }

    int appId = j["data"]["appId"].get<int>();
    std::string uid = j["data"]["uid"].get<std::string>();
    std::string extra = j["data"]["extra"].get<std::string>();

    // 打开数据库
    sqlite3 *db;
    int rc = sqlite3_open_v2("wxpusher_app_data.db", &db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                 SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_WAL,
                             nullptr);
    if (rc) {
        LOG_F(ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // 使用智能指针管理数据库资源
    std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> db_ptr(
        db, sqlite3_close_v2);

    // 创建表（如果不存在）
    const char *sql_create_table =
        "CREATE TABLE IF NOT EXISTS wxpusher_app_data ("
        "appId INTEGER,"
        "uid TEXT,"
        "extra TEXT);";
    char *err_msg = nullptr;
    rc = sqlite3_exec(db, sql_create_table, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_F(ERROR, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        evhttp_send_reply(req, 500, "Internal Server Error", buf);
        evbuffer_free(buf);
        return;
    }

    // 检查是否已经存在, 存在则覆盖
    std::string sql_query = "SELECT * FROM wxpusher_app_data WHERE appId = " +
                            std::to_string(appId) + " AND uid = '" + uid + "';";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr);

    // 使用智能指针管理预处理语句资源
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_ptr(
        stmt, sqlite3_finalize);

    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            LOG_F(INFO, "已存在，准备覆盖");

            // 更新数据
            std::string sql_update =
                "UPDATE wxpusher_app_data SET extra = '" + extra +
                "' WHERE appId = " + std::to_string(appId) + " AND uid = '" +
                uid + "';";
            rc = sqlite3_exec(db, sql_update.c_str(), nullptr, nullptr,
                              &err_msg);
            if (rc != SQLITE_OK) {
                LOG_F(ERROR, "SQL error: %s", err_msg);
                sqlite3_free(err_msg);
            }

            // 200 OK
            evhttp_send_reply(req, 200, "OK", buf);
            evbuffer_free(buf);
            return;
        }
    } else {
        LOG_F(ERROR, "SQL error: %s", sqlite3_errmsg(db));
        evhttp_send_reply(req, 500, "Internal Server Error", buf);
        evbuffer_free(buf);
        return;
    }

    // 插入数据
    std::string sql_insert =
        "INSERT INTO wxpusher_app_data (appId, uid, extra) VALUES (" +
        std::to_string(appId) + ", '" + uid + "', '" + extra + "');";
    rc = sqlite3_exec(db, sql_insert.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_F(ERROR, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
    }

    // 200 OK
    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
}

using list = std::vector<std::tuple<std::string, std::string>>;

list query_data_by_appid(int appid) {
    sqlite3 *db;
    list results;

    int rc = sqlite3_open_v2("wxpusher_app_data.db", &db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                 SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_WAL,
                             nullptr);
    if (rc) {
        LOG_F(ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return results;
    }

    // 使用智能指针管理数据库资源
    std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> db_ptr(
        db, sqlite3_close_v2);

    std::string sql_query =
        "SELECT uid, extra FROM wxpusher_app_data WHERE appId = " +
        std::to_string(appid) + ";";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {
        // 使用智能指针管理预处理语句资源
        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_ptr(
            stmt, sqlite3_finalize);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string uid =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string extra =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            results.emplace_back(uid, extra);
        }
    } else {
        LOG_F(ERROR, "SQL error: %s", sqlite3_errmsg(db));
    }

    return results;
}

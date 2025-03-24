#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <drogon/drogon.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <memory>

// 添加lib库，分debug和release
#ifdef _DEBUG
#pragma comment(lib, "lib/drogon_d.lib")
#pragma comment(lib, "lib/jsoncpp_d.lib")
#pragma comment(lib, "lib/trantor_d.lib")
#else
#pragma comment(lib, "lib/drogon.lib")
#pragma comment(lib, "lib/jsoncpp.lib")
#pragma comment(lib, "lib/trantor.lib")
#endif
#pragma comment(lib, "lib/mysqlcppconn.lib")

struct RelayData {
	std::string pin;
	std::string enumeration;
	int chainNumber; //This will need to be changed based on user input, Power = 0, Precision = 1
	int deviceNumber;
	std::string portNumber;
	std::string driveStrength;
	std::string normalState;
	int minOpenTime = 0; //This and all the other time values are hard coded in based on what numbers have worked for TI
	int typOpenTime = 60;
	int maxOpenTime = 200;
	int minCloseTime = 0;
	int typCloseTime = 100;
	int maxCloseTime = 500;
	std::string relayName;
};  //Holds all final relay data

struct NetlistData {
	std::vector<std::string> name;
	std::vector<std::string> identifier;
	std::vector<std::string> connection;
	std::vector<std::string> device;
};  //Holds all netlist input data

struct BillData {
	std::string item;
	std::string quantity;
	std::string manufacturer;
	std::string partNumber;
	std::vector<std::string> identifier;
	std::string description;
	std::string dni;
	std::string status;
	std::string maxTemp;
};  //Holds all BOM input data

struct IOData {
	std::string identifier;
	int position;
};  //Holds all IO expander data

struct ModelData {
	std::string relayModel;
	std::string driveStrength;
};  //Holds all information directly tied to the relay model


// 连接池 用于连接数据库
class MySQLConnectionPool {
public:
	MySQLConnectionPool(const std::string& host, int port, const std::string& user, const std::string& password, const std::string& schema, size_t poolSize)
		: host_(host), port_(port), user_(user), password_(password), schema_(schema), poolSize_(poolSize) {
		// 初始化 MySQL 驱动
		driver_ = sql::mysql::get_mysql_driver_instance();

		// 创建初始连接
		for (size_t i = 0; i < poolSize; ++i) {
			pool_.push(createConnection());
		}
	}

	std::shared_ptr<sql::Connection> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (pool_.empty()) {
			if (activeConnections_ < poolSize_) {
				activeConnections_++;
				return createConnection();
			}
			condition_.wait(lock);
		}
		auto conn = pool_.front();
		pool_.pop();
		return conn;
	}

	void releaseConnection(std::shared_ptr<sql::Connection> conn) {
		std::unique_lock<std::mutex> lock(mutex_);
		pool_.push(conn);
		condition_.notify_one();
	}

private:
	std::shared_ptr<sql::Connection> createConnection() {
		try {
			// 创建连接
			std::string connectionString = "tcp://" + host_ + ":" + std::to_string(port_);

			auto conn = std::shared_ptr<sql::Connection>(
				driver_->connect(connectionString, user_, password_),
				[](sql::Connection* conn) { conn->close(); delete conn; }
			);

			// 选择数据库
			conn->setSchema(schema_);
			return conn;
		}
		catch (const sql::SQLException& e) {
			std::cerr << "Failed to create MySQL connection: " << e.what() << std::endl;
			throw;
		}
	}

	sql::mysql::MySQL_Driver* driver_;  // MySQL 驱动
	std::queue<std::shared_ptr<sql::Connection>> pool_;  // 连接池
	std::mutex mutex_;  // 互斥锁
	std::condition_variable condition_;  // 条件变量
	size_t activeConnections_ = 0;  // 当前活跃连接数
	size_t poolSize_;  // 连接池大小

	std::string host_;  // 主机地址
	int port_;  // 端口
	std::string user_;  // 用户名
	std::string password_;  // 密码
	std::string schema_;  // 数据库名称
};
//#include "Input.h"
#include "Relay.h"
#include "Output.h"

void stopService(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
	auto resp = drogon::HttpResponse::newHttpResponse();
	resp->setBody("Service will stop.");
	callback(resp);
	drogon::app().quit();
}

void handleQuery(const std::shared_ptr<MySQLConnectionPool>& pool, const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
	try {
		// 解析 JSON 请求体
		std::string strBody = req->getBody().data();
		Json::Value jsonBody;
		Json::Reader reader;
		Input input;
		if (!reader.parse(strBody, jsonBody)) {
			auto resp = drogon::HttpResponse::newHttpResponse();
			resp->setStatusCode(drogon::k400BadRequest);
			resp->setBody("Invalid JSON format.");
			callback(resp);
			return;
		}
		if (jsonBody.isObject() && jsonBody["bom_path"].isString() && jsonBody["netlist_path"].isString()) {
			input.billName = jsonBody["bom_path"].asString();
			input.netlistName = jsonBody["netlist_path"].asString();
		}
		else {
			auto resp = drogon::HttpResponse::newHttpResponse();
			resp->setStatusCode(drogon::k400BadRequest);
			resp->setBody("BomPath or NetListPath empty.");
			callback(resp);
			return;
		}

		// 从连接池获取连接
		auto conn = pool->getConnection();

		// 初始化 Input 和 Output 对象
		Relay relay;
		Output output(conn);
		output.outputName = "Configuration.csv";

		// 从文件和数据库获取输入数据
		input.getInputData(input);

		// 将输入数据转换为继电器数据
		relay.relayConvert(input, relay);

		// 将继电器数据输出到数据库
		output.outputRelayData(relay, output);

		// 保存数据到数据库
		output.saveData(input, relay);

		// 释放连接
		pool->releaseConnection(conn);

		// 返回成功响应
		auto resp = drogon::HttpResponse::newHttpResponse();
		resp->setBody("relayAmount:" + std::to_string(relay.relayAmount));
		callback(resp);
	}
	catch (const sql::SQLException& e) {
		LOG_ERROR << "Database error: " << e.what();
		auto resp = drogon::HttpResponse::newHttpResponse();
		resp->setStatusCode(drogon::k500InternalServerError);
		resp->setBody("Database error: " + std::string(e.what()));
		callback(resp);
	}
	catch (const std::exception& e) {
		LOG_ERROR << "Error: " << e.what();
		auto resp = drogon::HttpResponse::newHttpResponse();
		resp->setStatusCode(drogon::k500InternalServerError);
		resp->setBody("Error: " + std::string(e.what()));
		callback(resp);
	}
}

int main() {
	try {
		// 初始化连接池 最后一个参数是连接池大小
		auto pool = std::make_shared<MySQLConnectionPool>("127.0.0.1", 3306, "haozi", "123456", "relay", 5);

		drogon::app().loadConfigFile("config.json");
		drogon::app().registerHandler("/stop", &stopService);

		// 注册 POST 请求处理函数
		drogon::app().registerHandler(
			"/query",
			[pool](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleQuery(pool, req, std::move(callback));
			},
			{ drogon::Post }
		);

		drogon::app().setLogLevel(trantor::Logger::kDebug);
		drogon::app().addListener("0.0.0.0", 18080);
		LOG_INFO << "Server started at 18080";
		drogon::app().run();
		LOG_INFO << "Server stopped.";
	}
	catch (const std::exception& e) {
		LOG_ERROR << "Exception: " << e.what();
		return 1;
	}

	return 0;
}

//int main() {
//	try {
//		auto driver = sql::mysql::get_driver_instance();
//		sql::ConnectOptionsMap connection_properties;
//		connection_properties["hostName"] = "localhost";
//		connection_properties["port"] = 3306;
//		connection_properties["userName"] = "root";
//		connection_properties["passWord"] = "123456";
//		connection_properties["dbname"] = "relay";
//
//		driver->connect(connection_properties);
//		std::cout << "Successfully connected to MySQL!" << std::endl;
//	}
//	catch (sql::SQLException& e) {
//		std::cerr << "SQL Error: " << e.what() << std::endl;
//	}
//	catch (std::runtime_error& e) {
//		std::cerr << "Runtime Error: " << e.what() << std::endl;
//	}
//	return 0;
//}
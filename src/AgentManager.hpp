#pragma once

#include <boost/asio.hpp>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "json.hpp"
#include "MySqlJdbcConnector.hpp"


using json = nlohmann::json;


class AgentManager
{
private:
	std::mutex m_control_mutex;
	std::thread m_main_thread;

	uint16_t m_discover_port;
	uint16_t m_server_port;

	MySqlJdbcConnector m_db;

	std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
	boost::asio::io_service m_io_service;

	std::map<std::string, std::unique_ptr<boost::asio::ip::tcp::socket>> m_connections;

	// If agent with that name doesn't exist, create a new record
	// If it does exist, update last_updated
	void addAgentToDb(const std::string &agent);
	bool updateAgentStatus(const std::string &agent, const std::string &status);

	static const int MAX_BUFFER_SIZE{ 1024 };
	// Seconds
	static const int AGENT_REFRESH_INTERVAL{ 5 };

public:
	AgentManager(uint16_t discover_port, uint16_t server_port);

	bool connectToDb(const std::string &xml_db_config);
	void discoverAgents();

	void run();
	void join();
	
	bool ping(const std::string &agent);
	void refresh();

	bool sendMessage(const std::string &agent, const std::string &msg);
	bool recvMessage(const std::string &agent, json &out);
	
	bool isConnected(const std::string &agent) const { return m_connections.count(agent); }
	void addConnection(const std::string &agent, std::unique_ptr<boost::asio::ip::tcp::socket> conn);
	std::string getAgentIp(const std::string &agent) const { return m_connections.at(agent)->remote_endpoint().address().to_string(); }
	std::vector<std::string> getAgents() const;
};

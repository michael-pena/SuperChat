#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"
#include "chat_message.hpp"
#include <fstream>
#include "display.h"

class Client{
public:
	  Client(asio::io_context& io_context,
		  const asio::ip::tcp::resolver::results_type& endpoints)
		: io_context_(io_context),
		  socket_(io_context)
	  {
		do_connect(endpoints);
	  }

	  void process_string(std::string s);

	  void set_display(Display *d){
		this->display = d;
	  }

	  void write(const chat_message& msg)
	  {
		asio::post(io_context_,
			[this, msg]()
			{
			  bool write_in_progress = !write_msgs_.empty();
			  write_msgs_.push_back(msg);
			  if (!write_in_progress)
			  {
				do_write();
			  }
			});
	  }

	  void close()
	  {
		asio::post(io_context_, [this]() { socket_.close(); });
	  }



private:
	  void do_connect(const asio::ip::tcp::resolver::results_type& endpoints)
	  {
		asio::async_connect(socket_, endpoints,
			[this](std::error_code ec, asio::ip::tcp::endpoint)
			{
			  if (!ec)
			  {
				do_read_header();
			  }
			});
	  }

	  void do_read_header()
	  {
		asio::async_read(socket_,
			asio::buffer(read_msg_.data(), chat_message::header_length),
			[this](std::error_code ec, std::size_t /*length*/)
			{
			  if (!ec && read_msg_.decode_header())
			  {
				do_read_body();
			  }
			  else
			  {
				socket_.close();
			  }
			});
	  }

	  void do_read_body()
	  {
		asio::async_read(socket_,
			asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this](std::error_code ec, std::size_t /*length*/)
			{
			  if (!ec)
			  {
			  std::ofstream ofile;
			  ofile.open("logfile", std::ofstream::app);
				ofile.write(read_msg_.body(), read_msg_.body_length());
				ofile << std::endl;
				ofile.close();
			
				if(read_msg_.body_length() == 512){//null terminate string 
					read_msg_.body()[511] = '\0';
				}else{
					read_msg_.body()[read_msg_.body_length()] = '\0';
				}

				process_message(std::string(read_msg_.body()));
				//std::cout.write(read_msg_.body(), read_msg_.body_length());
				//std::cout << "\n";
				do_read_header();
			  }
			  else
			  {
				socket_.close();
			  }
			});
	  }
public:
	  void process_message(std::string message);
private:
	  void process_regular(std::istringstream& iss);
	  void process_create(std::istringstream& iss);
	  void process_delete(std::istringstream& iss);
	  void process_register(std::istringstream& iss);
	  void process_login(std::istringstream& iss);
	  void print_chatrooms();
	  void print_users();

	  void do_write()
	  {
		asio::async_write(socket_,
			asio::buffer(write_msgs_.front().data(),
			  write_msgs_.front().length()),
			[this](std::error_code ec, std::size_t /*length*/)
			{
			  if (!ec)
			  {
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
				  do_write();
				}
			  }
			  else
			  {
				socket_.close();
			  }
			});
	  }

private:
	asio::io_context& io_context_;
	asio::ip::tcp::socket socket_;
	chat_message read_msg_;
	std::deque<chat_message> write_msgs_;
	Display *display;
};

#endif

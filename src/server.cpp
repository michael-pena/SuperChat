//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include "asio.hpp"
#include "chat_message.hpp"
#include <map>
#include <sstream>
#include "chatroom.h"
#include <time.h>

using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
	public:
		virtual ~chat_participant() {}
		virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
	public:
		void join(chat_participant_ptr participant)
		{
			participants_.insert(participant);
			for (auto msg: recent_msgs_){
				participant->deliver(msg);
			}
		}

		void leave(chat_participant_ptr participant)
		{
			participants_.erase(participant);
		}

		void deliver(const chat_message& msg)
		{
			//std::cout << msg.body() << std::endl;
			process_message(msg);
			recent_msgs_.push_back(msg);
			while (recent_msgs_.size() > max_recent_msgs)
				recent_msgs_.pop_front();

			//for (auto participant: participants_)
			//  participant->deliver(msg);
		}

		void process_message(chat_message msg){
			srand(time(NULL));
			msg.body()[msg.body_length()] = '\0';
			std::istringstream iss(msg.body());
			std::cout << "new message-\n\t" << msg.body() << std::endl;
			int operation = 0;
			iss >> operation;
			switch(operation){
				case 0:
					process_regular(iss);
					break;
				case 1:
					process_create(iss);
					break;
				case 2:
					process_delete(iss);
					break;
				case 4:
					process_register(iss);
					break;
				case 5:
					process_login(iss);
					break;
				default:
					std::cout << "Error: Didn't recognize opcode" << std::endl;
					break;
			}
			print_chatrooms();
			print_users();
			//broadcast_message("take me to your leader earthlings");
		}

		void process_regular(std::istringstream& iss){
			int chatroom;
			iss >> chatroom;
			std::cout << "got a message chatroom = " << chatroom << std::endl;
			std::cout << "chatrooms.size() == " << chatrooms.size() << std::endl;
			for(Chatroom& c : chatrooms){
				std::cout << "fwjeiognweroubnvewipoafhugbiuerodsijgnouqewrjf-ewvonaerufgnwuoerf" << chatrooms.size() << std::endl;
				std::cout << "current chatroom = " << c.id << std::endl;
				if(c.id == chatroom){
					std::cout << "found a room" << std::endl;
					c.messages.push_back(*new Message(iss));
					broadcast_message(iss.str());
					return;
				}
			}


			std::cout << "error couldn't find a chatroom to match the message " << chatroom << std::endl;
		}

		void process_create(std::istringstream& iss){
			Chatroom *newone = new Chatroom(iss);
			bool found = false;
			for(Chatroom& c : chatrooms)
				if(c.name == newone->name){
					found = true;
				}
			if(!found){
				chatrooms.push_back(*newone);
				broadcast_message(iss.str());
			}else{
				delete newone;
			}
		}

		void process_delete(std::istringstream& iss){
			int chatroom;
			iss >> chatroom;
			for(int i=0; i<(int)chatrooms.size(); i++){
				if(chatrooms[i].id == chatroom){
					chatrooms.erase(chatrooms.begin()+i);
				}
			}
			broadcast_message(iss.str());
		}

		void process_register(std::istringstream& iss){
			std::string name;
			int id;
			iss >> name >> id;
			id = rand();
			//std::cout << "id = " << id << std::endl;  
			bool found = false;
			for(auto u : users){
				if(u.second == name)
					found = true;
			}
			if(!found){
				users.emplace(id, name);
				broadcast_message("4 " + name + " " + std::to_string(id));
			}else{
				broadcast_message("4 " + name + " " + std::to_string(-1));
			}
		}

		void process_login(std::istringstream& iss){
			std::string name;
			int id;
			iss >> name >> id;
			if(users[id] == name){
				//send packet
				std::cout << "login credentials VERIFIED" << std::endl;
				broadcast_message("5 " + name + " " + std::to_string(1));
			}else{
				std::cout << "login credentials FAILED" << std::endl;
				broadcast_message("5 " + name + " " + std::to_string(0));
			}
		}

		void print_chatrooms(){
			std::cout << "-----------------------------------------------------" << std::endl;
			for( Chatroom c : chatrooms){
				std::cout << c.to_string();
			}
		}

		void print_users(){
			for( auto u : users){
				std::cout << u.first << " -> " << u.second << std::endl;
			}
			std::cout << "-----------------------------------------------------" << std::endl;
		}

		void broadcast_message(std::string message){
			assert(message.length() <=512);
			char line[512];
			strcpy(line,message.c_str());
			chat_message msg;
			msg.body_length(std::strlen(line));
			std::memset(msg.body(), '\0', 512);
			std::memcpy(msg.body(), line, msg.body_length());
			msg.encode_header();

			for (auto participant: participants_)
				participant->deliver(msg);
		}

	private:
		std::set<chat_participant_ptr> participants_;
		enum { max_recent_msgs = 100 };
		chat_message_queue recent_msgs_;
		std::vector<Chatroom> chatrooms;
		std::map<int, std::string> users;

};

//----------------------------------------------------------------------

class chat_session
: public chat_participant,
	public std::enable_shared_from_this<chat_session>
{
	public:
		chat_session(tcp::socket socket, chat_room& room)
			: socket_(std::move(socket)),
			room_(room)
	{
	}

		void start()
		{
			room_.join(shared_from_this());
			do_read_header();
		}

		void deliver(const chat_message& msg)
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);
			if (!write_in_progress)
			{
				do_write();
			}
		}

	private:
		void do_read_header()
		{
			auto self(shared_from_this());
			asio::async_read(socket_,
					asio::buffer(read_msg_.data(), chat_message::header_length),
					[this, self](std::error_code ec, std::size_t /*length*/)
					{
					if (!ec && read_msg_.decode_header())
					{
					do_read_body();
					}
					else
					{
					room_.leave(shared_from_this());
					}
					});
		}

		void do_read_body()
		{
			auto self(shared_from_this());
			asio::async_read(socket_,
					asio::buffer(read_msg_.body(), read_msg_.body_length()),
					[this, self](std::error_code ec, std::size_t /*length*/)
					{
					if (!ec)
					{
					room_.deliver(read_msg_);
					do_read_header();
					}
					else
					{
					room_.leave(shared_from_this());
					}
					});
		}

		void do_write()
		{
			auto self(shared_from_this());
			asio::async_write(socket_,
					asio::buffer(write_msgs_.front().data(),
						write_msgs_.front().length()),
					[this, self](std::error_code ec, std::size_t /*length*/)
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
					room_.leave(shared_from_this());
					}
					});
		}

		tcp::socket socket_;
		chat_room& room_;
		chat_message read_msg_;
		chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server
{
	public:

		chat_server(asio::io_context& io_context,
				const tcp::endpoint& endpoint)
			: acceptor_(io_context, endpoint)
		{
			do_accept();
		}

	private:
		void do_accept()
		{
			acceptor_.async_accept(
					[this](std::error_code ec, tcp::socket socket)
					{
					if (!ec)
					{
					std::make_shared<chat_session>(std::move(socket), room_)->start();
					}

					do_accept();
					});
		}

		tcp::acceptor acceptor_;
		chat_room room_;
};

//----------------------------------------------------------------------


int main(int argc, char* argv[])
{
	try
	{
		if (argc < 2)
		{
			std::cerr << "Usage: chat_server <port> [<port> ...]\n";
			return 1;
		}

		asio::io_context io_context;
		std::list<chat_server> servers;
		for (int i = 1; i < argc; ++i)
		{
			tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
			servers.emplace_back(io_context, endpoint);
		}

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}


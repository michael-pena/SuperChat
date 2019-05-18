#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <iostream>
#include <string>
#include <vector>
#include "chatroom.h"
#include <fstream>
#include <iostream>

class Client;

class Display{
	public:
		Display(void(Client::*cb)(std::string), Client& c) : user("guest"), callback(cb), client(c) {};
		void Init();
		void Welcome_Window();
		void Update_Input();
		void Update();
		void End_Display();
		void Get_User_Input();
		void Choose_Chatroom(WINDOW *lobby_window, int highlight);

		std::vector<Chatroom> chatrooms;
		std::vector<std::string> blocked_users;

		std::string user;
		int current_room;
		std::string current_input;//input currently been typed by the user
		std::string current_status;
		bool changed;//if this is set the screen needs to be redrawn
		std::string pending_register;
		std::string pending_login;

	private:

		int x_max, y_max;
		WINDOW*  status_window ; //(length, width,lines_down, lines to right)
		WINDOW*  lobby_window ; //(length, width,lines_down, lines to right)
		WINDOW*  welcome_window ; //(length, width,lines_down, lines to right)
		WINDOW*  chat_window ; //(length, width,lines_down, lines to right)
		WINDOW*  chat_input_window ; //(length, width,lines_down, lines to right)

		void (Client::* callback)(std::string);//callback is a callback now
		Client& client;
};

#endif

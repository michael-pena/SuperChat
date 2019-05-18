#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <random>
#include "client.h"
#include "display.h"

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: chat_client <host> <port>\n";
		return 1;
	}

	asio::io_context io_context;
	asio::ip::tcp::resolver resolver(io_context);
	auto endpoints = resolver.resolve(argv[1], argv[2]);
	Client c(io_context, endpoints);
	std::thread t([&io_context](){ io_context.run(); });

	Display d1(&Client::process_string, c);
	c.set_display(&d1);


	d1.Init();
	d1.Welcome_Window();

	bool lobby_found = false;

	for (int i = 0; i < (int)d1.chatrooms.size(); i++)
	{
		if (d1.chatrooms[i].name == "Lobby")
		{
			lobby_found = true;
		}
	}

	if (lobby_found == false)
	{
		c.process_string("/create Lobby");
	}



	//c.process_string("/create Lobby");

	d1.Update();

	while(true){
		d1.Get_User_Input(); usleep(10);
	}

	d1.End_Display();

	c.close();
	t.join();
}

void Client::process_string (std::string s){
	//std::cout << s; //prints input
	srand(time(0));
	int chat_id = rand(); //gets the random chat id using time

	//tokenizes the user input and stores it into a vector called vstrings
	std::stringstream ss(s);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> vstrings(begin, end);

	//stores the input in the example file for testing & debugging
	std::ofstream output_file("./example.txt");
	std::ostream_iterator<std::string> output_iterator(output_file, "\n");
	std::copy(vstrings.begin(), vstrings.end(), output_iterator);
	if(vstrings.size() == 0)
		return;

	if (vstrings[0] == "/create") //creates a chatroom
	{
		s = "1 " + vstrings[1] + " " + std::to_string(chat_id); //op_code chatroom_name chatroom_id
		//output_file << s;
	}
	else if (vstrings[0] == "/delete") //this deletes the chatroom id
	{
		s = "2 " + vstrings[1]; //op_code chatroom_id
		//output_file << s;
	}
	else if (vstrings[0] == "/register")
	{
		s = "4 " + vstrings[1] + " 0"; // op_code name 0
		display->pending_register = vstrings[1];
		//output_file << s;
	}
	else if (vstrings[0] == "/login")
	{
		if (vstrings.size() < 3)
		{
			display->current_status = "Please enter correct number args.";
			return;
		}

		s = "5 " + vstrings[1] + " " + vstrings[2]; //op_code name id_num
		display->pending_login = vstrings[1];
		//output_file << s;
	}
	else if (vstrings[0] == "/block")
	{
		if (vstrings.size() < 2)
		{
			display->current_status = "Please enter correct number args.";

			return;
		}

		//arg 1 vstrings username to block
		display->blocked_users.push_back(vstrings[1]);
		display->current_status = "Blocked user" + vstrings[1]+".";
	}
	else if (vstrings[0] == "/exit")
	{
		endwin();
		exit(0);
	}
	else if (vstrings[0] == "/help")
	{
		int y_max,x_max;
		getmaxyx(stdscr, y_max,x_max);
		WINDOW *welcome_window = newwin(y_max,x_max,0,0); //(length, width,lines_down, lines to right)
		box(welcome_window,0,0);

		mvwprintw(welcome_window,1,((x_max/2)-12),"Welcome to SuperChat");
		mvwprintw(welcome_window,4,1,"Insructions:");
		mvwprintw(welcome_window,5,3,"Sign in with /register and enter a valid username to register with the server.");
		mvwprintw(welcome_window,6,1,"Then start typing your message to start chatting.");
		mvwprintw(welcome_window,8,1,"Commands:");
		mvwprintw(welcome_window,9,3,"To login - /login <username> <id#>");
		mvwprintw(welcome_window,10,3,"To create a Chatroom - /create <chatroom name>");
		mvwprintw(welcome_window,11,3,"To show this screen again - /help <enter>");
		mvwprintw(welcome_window,12,3,"To block a user - /block <username>");
		mvwprintw(welcome_window,13,3,"To exit - /exit");

		mvwprintw(welcome_window,(y_max-2),((x_max/2)-12),"Press any key to continue");
		getch();
		wrefresh(welcome_window);
		getch();
		delwin(welcome_window);
	}
	else
	{
		s = "0 " + std::to_string(display->chatrooms[display->current_room].id) + " " + display->user + " " + std::to_string(time(NULL)) + " " + s;
	}
	//output_file << s;

	//store in to line.
	char line[512];
	strncpy(line, s.c_str(), 512); //copies to line
	chat_message msg;

	//if
	//sends message
	msg.body_length(std::strlen(line));
	std::memset(msg.body(), '\0', 512);
	std::memcpy(msg.body(), line, msg.body_length());
	msg.encode_header();
	//output_file << "\n\nsending message-\n\t" << msg.body() << std::endl;
	this->write(msg);
	vstrings.clear();
}

void Client::process_message(std::string message){
	srand(time(NULL));
	std::istringstream iss(message);
	int operation;
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
	display->changed = true;
	//print_users();
	//broadcast_message("take me to your leader earthlings");
}

void Client::process_regular(std::istringstream& iss){
	int chatroom;
	iss >> chatroom;
	for(Chatroom& c : display->chatrooms){
		if(c.id == chatroom){
			c.messages.push_back(*new Message(iss));
			return;
		}
	}
	//std::cout << "error couldn't find a chatroom to match the message " << chatroom << std::endl;
}

void Client::process_create(std::istringstream& iss){
	display->chatrooms.push_back(*new Chatroom(iss));
}

void Client::process_delete(std::istringstream& iss){
	int chatroom;
	iss >> chatroom;
	for(int i=0; i<(int)display->chatrooms.size(); i++){
		if(display->chatrooms[i].id == chatroom){
			display->chatrooms.erase(display->chatrooms.begin()+i);
		}
	}
}

void Client::process_register(std::istringstream& iss){
	std::string name;
	int id;
	iss >> name >> id;

	if(display->pending_register == name){
		if(id == -1){
			display->current_status = "Sorry, someone has already registered that username";
			display->pending_login = "";
		}else{
			display->user = name;
			display->current_status = "Your password for \"" + name + "\" is " + std::to_string(id);
			display->pending_login = "";
		}
	}
}

void Client::process_login(std::istringstream& iss){
	std::string name;
	int id;
	iss >> name >> id;
	if(name == display->pending_login){
		if(id){
			display->current_status = "Successfully logged in as \"" + name + "\"";
			display->user = name;
			display->pending_login = "";
		}else{
			display->current_status = "Login FAILED";
			display->pending_login = "";
		}
	}
}

void Client::print_chatrooms(){
	std::ofstream ofile;
	ofile.open("logfile", std::ofstream::app);
	//ofile.write(read_msg_.body(), read_msg_.body_length());
	//ofile << std::endl;
	ofile << "-----------------------------------------------------" << std::endl;
	for( Chatroom c : display->chatrooms){
		ofile << c.to_string();
	}
	ofile.close();
}

void Client::print_users(){
	//	for( auto u : users){
	//		std::cout << u.first << " -> " << u.second << std::endl;
	//	}
	//	std::cout << "-----------------------------------------------------" << std::endl;
}

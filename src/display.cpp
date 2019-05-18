#include "display.h"
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <algorithm>

/*
 * Todo: make messagebox 1 row high
 *       make password a string
 *       update welcome screen
 */

void Display::Init()
{
	initscr();
	curs_set(1);
	cbreak();
	noecho();

	current_room = 0;
	current_input = "";

  	getmaxyx(stdscr, y_max,x_max);

	//Welcome WINDOW*;
	status_window = newwin(3,x_max,y_max-3,0); //(length, width,lines_down, lines to right)
	lobby_window = newwin(y_max-3,15,0,0); //(length, width,lines_down, lines to right)
	chat_window = newwin(y_max-9,x_max-15,0,15); //(length, width,lines_down, lines to right)

	chat_input_window = newwin(6,x_max-15 ,y_max-9,15); //(length, width,lines_down, lines to right)
	nodelay(chat_input_window, TRUE);
	keypad(chat_input_window, TRUE);

}

void Display::Welcome_Window(){
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
		//wclear(welcome_window);
		delwin(welcome_window);
}

void Display:: Update_Input(){
		wclear(chat_input_window);
		box(chat_input_window,0,0);
		mvwprintw(chat_input_window,1,1,(user+": "+current_input).c_str()); //replace with variable for username
		wrefresh(chat_input_window);
}

void Display::Update(){
		wclear(status_window);
		box(status_window,0,0);
		mvwprintw(status_window,1,1,"Status: %s", current_status.c_str());
		wrefresh(status_window);

		wclear(lobby_window);
		box(lobby_window,0,0);
		int i=0;
		for(Chatroom& c : chatrooms){
			if(i>10) break;// just in case
			  mvwprintw(lobby_window,1+i,1,c.name.c_str());
			  i++;
		}
		for(int i=0; i<(int)chatrooms.size(); i++){
			if(i>10) break;// just in case
			if(i == current_room){
				wattron(lobby_window, A_STANDOUT);
				mvwprintw(lobby_window,1+i,1,chatrooms[i].name.c_str());
				wattroff(lobby_window, A_STANDOUT);
			}else{
				mvwprintw(lobby_window,1+i,1,chatrooms[i].name.c_str());
			}
		}
		wrefresh(lobby_window);

		wclear(chat_window);
		box(chat_window,0,0);
		int chat_x, chat_y;
		getmaxyx(chat_window, chat_y,chat_x);
		chat_y-=2;
		for(i=chatrooms[current_room].messages.size()-1; i>=0; i--){
			if(blocked_users.end() == std::find(blocked_users.begin(), blocked_users.end(), chatrooms[current_room].messages[i].author))
			{mvwprintw(chat_window, chat_y, 1, chatrooms[current_room].messages[i].to_string().c_str());//mouthful
			chat_y--;}
		}
		wrefresh(chat_window);

		Update_Input();
}

void Display::Get_User_Input(){
	if(changed){
		Update();
		changed=0;
	}

	std::ofstream ofile;
	ofile.open("logfile", std::ofstream::app);
	int c;

	mvwprintw(chat_input_window,1,1,(user+": "+current_input).c_str()); //replace with variable for username
	wmove(chat_input_window, 1, (user+": "+current_input).length()+1);

	while((c = wgetch(chat_input_window)) != ERR){

		switch(c){
			case '\n':
				(client.*callback)(current_input);//call the callback function to client with the text the user entered
				current_input = "";
				Update();
				break;
			case 127: //backspace
				//ofile << "backspace" << std::endl;
				//ofile.flush();
				current_input = current_input.substr(0,current_input.size()-1);
				Update_Input();
				break;
			case 263: //backspace
					//ofile << "backspace" << std::endl;
					//ofile.flush();
				current_input = current_input.substr(0,current_input.size()-1);
				Update_Input();
				break;
			case KEY_DOWN:
				if(current_room >= (int)chatrooms.size()-1){
					current_room = chatrooms.size()-1;
				}else{
					current_room++;
				}
				////ofile << "keydown" << current_room << "/" << chatrooms.size()-1 << std::endl;
				//ofile.flush();
				Update();
				break;
			case KEY_UP:
				if(current_room <= 0 ){
					current_room = 0;
				}else{
					current_room--;
				}
				//ofile << "keyup" << current_room << "/" << chatrooms.size()-1 << std::endl;
				//ofile.flush();
				Update();
				break;
			default:
				ofile << "recieved "<< c << std::endl;
				ofile.flush();
				current_input += (char)c;
				Update_Input();
				break;
		}
	}
}


void Display::End_Display()
{
	endwin();
}

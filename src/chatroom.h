#include <string>
#include "message.h"

class Chatroom
{
	public:
		Chatroom(std::istringstream& iss){
			iss >> name >> id;
		}
		std::string to_string(){
			std::ostringstream oss;
			oss << "Chatroom #" << id << "  \"" << name << "\"" << std::endl;
			for(Message m : messages){
				oss << "\t" << m.to_string() << std::endl;
			}
			return oss.str();
		}

		std::string name;
		std::vector<Message> messages;
		int id;

};

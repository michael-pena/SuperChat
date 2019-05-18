#include <string>
#include <sstream>
#include <time.h>

class Message{
	public: 
		Message(std::string a, int t, std::string txt) : author(a), time(t), text(txt){}
		Message(std::istringstream& iss){
			
			iss >> author >> time;
			iss >> std::noskipws;
			char a;
			iss >> a;
			while(iss){
				text += a;
				iss >> a;
			}
		}
		std::string to_string(){
			time_t t = (time_t)time;
			struct tm *tm = localtime(&t);
			char date[20];
			strftime(date, sizeof(date), "%H:%M", tm);

			return "[" + author + " @ " + date + "]" + " -" + text;
		}

		std::string author;
		int time;
		std::string text;
};

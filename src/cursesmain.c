#include <curses.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  initscr();
  noecho();
  curs_set(FALSE);

  int x_max, y_max;
  getmaxyx(stdscr, y_max,x_max);

  while(1)
  {
    sleep(1);
    //create a window for status
    WINDOW*  status_window = newwin(3,x_max,y_max-3,0); //(length, width,lines_down, lines to right)
    box(status_window,0,0);
    mvwprintw(status_window,1,1,"status:");

    //create window for lobby
    WINDOW*  lobby_window = newwin(y_max-3,15,0,0); //(length, width,lines_down, lines to right)
    mvwprintw(lobby_window,1,1,"Lobby");
    box(lobby_window,0,0);

    //create chat window
    WINDOW*  chat_window = newwin(y_max-9,x_max-15,0,15); //(length, width,lines_down, lines to right)
    mvwprintw(chat_window,1,1,"User1: helloworld");
    box(chat_window,0,0);

    //create a window for chat input
    WINDOW*  chat_input_window = newwin(6,x_max-15 ,y_max-9,15); //(length, width,lines_down, lines to right)
    box(chat_input_window,0,0);
    mvwprintw(chat_input_window,1,1,"user1:");

    wrefresh(lobby_window);
    wrefresh(status_window);
    wrefresh(chat_window);
    wrefresh(chat_input_window);
  }
    getch();

    endwin();
    return 0;
}

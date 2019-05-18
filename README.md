# SuperChat
![alt tag](https://raw.githubusercontent.com/michael-pena/SuperChat/master/welcome_screen.png)
![alt tag](https://raw.githubusercontent.com/michael-pena/SuperChat/master/Chat_Screen.png)

<h3>What is SuperChat?</h3>
SuperChat is a chat program written in c++ and is my final project for 3310 Fundamentals Of Software Engineering class. It uses ncurses to display the interface and uses asio for socket handling. <br><br>

<h3>How to use it?</h3>
Usage is simple and Superchat has an easy to use interface despite the use of ncurses. The commands are printed at the welcome screen and to pull up the commands any time, just type "/help".

<h3>Dependencies?</h3>
<ul>
<li> <code> sudo apt-get install libncurses5-dev libncursesw5-dev </code></li>
<li> <a href="https://think-async.com/Asio/">boost</a> </li>
</ul>

<h3>How to run?</h3>
make <br>
./build/server (port number) <br>
./build/client 127.0.0.1 (port number) <br>

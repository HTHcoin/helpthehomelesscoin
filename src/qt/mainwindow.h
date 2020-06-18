#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
using namespace std;

class User {

public:
    string username;
    string password;
    string inputUsername;
    string inputPassword;
    
    // User registration
    void userRegisterDo()
    {

        ofstream usersFile ("userData.txt");

        if (	!usersFile.is_open())
        {
            usersFile.open("userData.txt");
        }

            usersFile << username << " " << password << endl;

            usersFile.close();
    }

    // Initialize user registration
    void userRegister()
    {
        cout << "Welcome!\n-------------------------\n\nPlease register.\nEnter a new username:\n";
        cin >> username;

        cout << "\nPlease enter a new password:\n";
        cin >> password;

        userRegisterDo();
    }

    // User login function
    void login()
    {
        cout << "Please enter your username:\n";
        cin >> inputUsername;

        cout << "\nPlease enter your password:\n";
        cin >> inputPassword;

        string userAndPass = inputUsername + " " + inputPassword; // Search pattern
        int offset; 
        string line;
        ifstream usersFile;
        usersFile.open ("userData.txt");

       bool found = false;
      if(usersFile.is_open()) {
          while(getline(usersFile,line) && !found) {
              if (line.compare(userAndPass) == 0) { //match strings exactly!
                  found = true; // found is true => break loop
              }
          }
	usersFile.close(); //close the file before recursivly opening it again later
	if(found) {
		cout << "Welcome "<< inputUsername << '\n';
	}
	else {
		cout << "\nUsername and/or password incorrect!\n\n\n\n";
		login(); //ok we didn't find them, lets redue this!
	}
      }
    }
  }

// Main program
int main() {
    
    User user1;
    ifstream usersFile("userData.txt");
    long begin, end;

    if (usersFile.good())
    {
        cout << "File userData.txt found!\n\n";
    }
    
    else {
        user1.userRegister();
    }

    if(usersFile.is_open())
    {
        begin = usersFile.tellg();
        usersFile.seekg (0, ios::end);

        end = usersFile.tellg();
        usersFile.close();

        if(begin == end)
        {
            user1.userRegister();
        }

        else
            {
                user1.login();
            }
    }

    getch();
}

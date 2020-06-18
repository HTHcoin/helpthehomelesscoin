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

        if(usersFile.is_open())
        {
            while(!usersFile.eof())
            {
                getline(usersFile,line);
                if ((offset = line.find(userAndPass, 0)) != string::npos) {	  // if login data is found
                    cout << "found " << userAndPass << endl;
                }

                else	// if login data is not found
                {
                    cout << "\nUsername and/or password incorrect!\n\n\n\n";
                    login();
                }
            }
            
			usersFile.close();
        }
        else
            cout << "Unable to open userData.txt file." << endl;
    }

};

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

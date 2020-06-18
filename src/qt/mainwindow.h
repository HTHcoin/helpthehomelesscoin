#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> // for exit(1);

using namespace std;


void Login();

int main()
{
	Login();
	return 0;
}

void Login()
{
	char login_un[50], login_pw[50], username[50], password[50];
	int c;
	ifstream uin("user.txt");
	ifstream pin("pass.txt");
	
	cout<<"Main\n\n"
		<<"(1) Login\n"
		<<"(2) Quit\n";
	cin>> c;
	
	
	
	if (c==1)
	{
		uin.getline(username, 50);
		while (strcmp(login_un, username) !=0)
		{
			cout<<"Username: ";
			cin.getline(login_un, 50);
			if (strcmp(login_un, username) ==0) break;
			else
				cout<<"\nInvalid Username.\n";
		}
		
		pin.getline(password, 50);
		while (strcmp(login_pw, password) !=0)
		{
			cout<<"\nPassword: ";
			cin.getline(login_pw, 50);
			if (strcmp(login_pw, password) ==0) break;
			else
				cout<<"\nInvalid Password\n";
		}
		
	}
	
	else if (c==2)
	{
		cout<<"Quitting\n";
		exit(1);
	}
	return;
}

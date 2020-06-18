#include<iostream>//.h>
#include<string.h>
//#include<conio.h>

using namespace std; ///

int main(){

	char Username[30]="sammasangcap",Password[30]="abc123",username[30], password[30],age[2],name[60],sex[6],address[100];

    unsigned int attempts = 0;
    do
    {
        cout<<"\nUsername:";
        cin>>username;

        if(strlen(username) <4)
        {
            cout<<"\nUsername length must be atleast 4 characters";
        }
        else
        {
            cout<<"\nPassword:";
            cin>>password;
        }
       

        if(!( (strcmp(username,Username) == 0) && (strcmp(password,Password) == 0)))
        {
            cout<<"\nWrong username or password";
            attempts++;
            if(3 == attempts)
            {
                cout<<"You exceeded the number of times that you need to input the correct username and password. The system now is locked!";
                return 0;
            }

        }
        else
        {
            cout<<"\nLogin Successful!";
            cout<<"\nPlease input the following information:";
            cout<<"\nName";
            cin>>name;
            cout<<"\nAge:";
            cin>>age;
            cout<<"\nSex:";
            cin>>sex;
            cout<<"\nAddress:";
            cin>>address;
            break;
        }
    }while(1);

	//getch();
	return 0;


}

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

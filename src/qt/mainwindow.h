#include<iostream>
#include<string.h>
#include<stdio.h>
#include<ctype.h>

using namespace std;

int main()
{
     char uname[50];
     char pass[20];
     cout<<"\n Enter User Name  :  ";
     gets(uname);
     cout<<"\n Enter Password   :  ";
     gets(pass);
     try
     {
          //This section is used to verify the number of characters.
          if(strlen(pass)<6)
          {
               cout<<"\n Password must be at least 6 Characters Long..."<<endl;
               throw 'c';
          }
          //This section is used to verify the digits in the string.
          bool digit_yes=false;
          bool valid;
          int len = strlen(pass);
          for (int count = 0; count < len; count++)
          if (isdigit(pass[count]))      
               digit_yes=true;
          if (!digit_yes)
          {
               valid=false;
               cout <<"\n Password must have at least One Digit (0-9)"<< endl;
               throw 'c';
          }
          else
          {
               valid=true;
               cout<<"\n Password is Correct";
          }
     }
     catch(char c)
     {
          cout<<"\n Invalid Password Format!!!";
     }
     catch(...)
     {
          cout<<"\n Default Exception";
     }
     return 0;
}

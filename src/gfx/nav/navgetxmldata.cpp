#include "navgetxmldata.h"





string retrievedata(string data, string type)
{
	int length = data.size();
	if(length == 0)
		return "";


	if(type.size() == 0)
		return "";

	int testlength = type.size();
	char testchar = 'p';
	string teststring = "";

	int counter = 0;
	while(counter < length)
	{
		if(data[counter] == '=')	//	find an =
		{
			int tempcounter = counter;
			teststring = "";

			while(tempcounter > 0)	//	count backwards to find its start
			{
				tempcounter = tempcounter - 1;
				testchar = data[tempcounter];

				if((testchar == '"')||(tempcounter == 0)||(testchar == ' '))	//	found its start
				{
					if(tempcounter == 0)
						teststring = testchar + teststring;

				//	if(testchar == ' ')
				//		teststring = ' ' + teststring;

					//	kill spaces at front and back
					//	***********************************
					int startspaces = 0;
					int endspaces = teststring.size()-1;
					string possibletype = "";

					while(teststring[startspaces] == ' ')
						startspaces += 1;

					while(teststring[endspaces] == ' ')
						endspaces -= 1;


					for(int j = startspaces; j <= endspaces; j++)
					{
						possibletype = possibletype + teststring[j];
					}
					//	***********************************



					//	if match, return the data afterwards
					//	***********************************
					if(possibletype == type)
					{
				//		cout << possibletype;
						int returncounter = counter;
						string returnstring = "";
						returncounter += 1;	//	pass the =

						while(data[returncounter] == ' ')
							returncounter += 1;

						returncounter += 1;	//	pass the "

						while(data[returncounter] == ' ')	//	pass starting spaces in answer
							returncounter += 1;

						while(returncounter < (data.size()-1))
						{
							if((data[returncounter] == '"')||(data[returncounter] == ' '))	//	read upto the second comment
								break;

							returnstring = returnstring + data[returncounter];
							returncounter += 1;
						}


						return returnstring;
					}
					//	***********************************
					else
					{
						break;
					}

				break;
				}


				teststring = testchar + teststring;
			}
		}

		counter += 1;
	}



	return ("");
};

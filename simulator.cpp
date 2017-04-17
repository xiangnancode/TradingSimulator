#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "curl/curl.h"
#include <ctime>
#include <unordered_map>

using namespace std;

struct Userinfo {
	string name;
	double fund;
	unordered_map<string, int> accountdetial;
};

int writer(char *data, size_t size, size_t nmemb, string *buffer){
	int result = 0;
	if(buffer != NULL) {
		buffer -> append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
} 

string getprice(string rawdata) {
  string res;
  istringstream rawdatastream(rawdata);
  string ticker, entry;
  while (getline (rawdatastream, ticker, '}'))
  {
    istringstream tickerstream(ticker);
    getline(tickerstream, entry, 'l');
    getline(tickerstream, entry, '"');
    getline(tickerstream, entry, '"');
    if(getline(tickerstream, entry, '"')) {
      //res += ',';
      res.append(entry);
    }
  }
  res += '\n';
  return res;
}

string getonlinedata(string tickers){
  /* (A) Variable Declaration */
  CURL *curl;   /* That is the connection, see: curl_easy_init */
  string buffer;    /* See: CURLOPT_WRITEDATA */
  string url = "http://finance.google.com/finance/info?q="+tickers;
  
  /* (B) Initilise web query */
  curl = curl_easy_init();
  
  /* (C) Set Options of the web query 
   * See also:  http://curl.haxx.se/libcurl/c/curl_easy_setopt.html  */
  if (curl){
    //curl_easy_setopt(curl, CURLOPT_URL, "http://ichart.finance.yahoo.com/table.csv?s=DAI.DE&a=NaN&b=02&c=pr-2&g=d&ignore=.csv");
    curl_easy_setopt(curl, CURLOPT_URL, &url[0]);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);   /* No we don't need the Header of the web content. Set to 0 and curl ignores the first line */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0); /* Don't follow anything else than the particular url requested*/
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);  /* Function Pointer "writer" manages the required buffer size */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer ); /* Data Pointer &buffer stores downloaded web content */ 
  }
  else{
    fprintf(stderr,"Error 1\n");  /* Badly written error message */
    return 0;                     
  }
  
  /* (D) Fetch the data */
    curl_easy_perform(curl);
  
  /* (E) Close the connection */
  curl_easy_cleanup(curl);
  return getprice(buffer);
}
string to_uppercase(string s) {
	for(int i = 0; i < s.size(); i++) {
	    s.at(i) = toupper(s.at(i));
	}
	return s;
}

void outputtradelog(string filename, string entry) {
  ofstream file;
  file.open("./usersinfo/" + filename + "_trading_log", std::ios_base::app);
  if (!file.is_open()) {
  	cout << "file did not open.\n";
  }
  file << entry;
  file.close();
}

void trade(Userinfo &user) {
	bool process = false; //trade command indicator
	string buyorsell, symb;
	int nshares;
	char confirm;
	while(!process) {
		cout << "Input format:" << endl;
		cout << "buy/sell symb #share(s)" << endl;
		cin >> buyorsell >> symb >> nshares;
		symb = to_uppercase(symb);
		if (buyorsell.compare("buy") && buyorsell.compare("sell")) {
			cout << "Input error, please re-enter" << endl;
			continue;
		}else if (getonlinedata(symb) == "\n") {
			cout << "symb did not find, please re-enter" << endl;
			continue;
		}
		cout << buyorsell << "ing " << nshares << " share(s) of " << symb << endl;
		cout << "Please confirm: Y/N? ";
		cin >> confirm;
		if (confirm == 'Y') {
			process = true;
		} else {
			cout << endl << "Please re-enter..." << endl;
		}
		
	}
	cout << "Processing..." << endl;
	double price = stod(getonlinedata(symb));//get real time price
	double totalprice = price * (double) nshares;
	cout << "\n" << buyorsell << "ing " << nshares << " share(s) of " << (symb)  << " at $" << to_string(price) << ".\n\n";
	// if buying
	if (!buyorsell.compare("buy")) { 
		if (user.fund < price * (double) nshares) {// if no enough fund to buy
			cout << "Need " << to_string(totalprice) << " to buy, but you only have " << to_string(user.fund) << ".\n";
			cout << "Enter Y to buy as much as you can (" << (int) (user.fund / price) << " shares), or enter N to terminate the trade.\n";
			cin >> confirm;
			if (confirm == 'Y') {
				nshares = (int) (user.fund / price);
				totalprice = price * (double) nshares;
				cout << "\n" << buyorsell << "ing " << nshares << " share(s) of " << (symb)  << " at $" << to_string(price) << ".\n";
			} else {
				return;//terminate
			}
		}
		if (user.accountdetial.find(symb) == user.accountdetial.end()) {
			user.accountdetial[symb] = nshares;
		} else {
			user.accountdetial[symb] += nshares;
		}
		user.fund -= totalprice;
	}
	// if selling
	if (!buyorsell.compare("sell")) { 
		if (user.accountdetial.find(symb) == user.accountdetial.end()) {// check if holding
			cout << "You are not holding " << symb << ".\nTrade terminated.";
			return;//terminate
		} else if (user.accountdetial[symb] < nshares) {// if no enough share to sell
			cout << "You only hold " << user.accountdetial[symb] << " shares.\n";
			cout << "Enter Y to sell them all, or enter N to terminate the trade.\n";
			cin >> confirm;
			if (confirm == 'Y') {
				nshares = user.accountdetial[symb];
				cout << "\n" << buyorsell << "ing " << nshares << " share(s) of " << (symb)  << " at $" << to_string(price) << ".\n";
				totalprice = price * (double) nshares;
			} else {
				return;//terminate
			}
		}
		if (nshares == user.accountdetial[symb]) {
			user.accountdetial.erase(symb);
		} else {
			user.accountdetial[symb] -= nshares;
		}
		user.fund += totalprice;
	}
	//get current time
	time_t now;
	tm *gmtm;
	now = time(0);
	gmtm = gmtime(&now);
	string tradelog = to_string(gmtm->tm_mon + 1) + "-" + to_string(gmtm->tm_mday) + "-" + to_string(gmtm->tm_year + 1900) + ",";
	tradelog += to_string(gmtm->tm_hour - 4) + ":" + to_string(gmtm->tm_min) + ":" + to_string(gmtm->tm_sec);
	//
	int side = buyorsell.compare("buy") ? -1 : 1;
	tradelog += "," + (symb) + "," + to_string(side) + "," + to_string(nshares) + "," + to_string(price) + "\n";
	//output trade log
	cout << "\ntrade log: " << tradelog << endl;
	outputtradelog(user.name, tradelog);
	//return tradelog;
}

bool checkusername(string username) {
	ifstream file ("./usersinfo/usersinfo");
	string entry;
	while (file.good()) 
	{
	  getline(file, entry,'[');
	  getline(file, entry,'\n');
	  getline(file, entry,'\n');
	  if (!username.compare(entry)) {
	  	file.close();
	  	return true;
	  }
	}
	file.close();
	return false;
}

void creataccount(string username, string password, double fund) {
	cout << "creating account...\n";
	ofstream file;
  	file.open("./usersinfo/usersinfo", std::ios_base::app);
  	file << "[\n" + username + "\n" + password + "\n]\n";
  	file.close();
  	file.open("./usersinfo/" + username + "_account_detial", std::ios_base::app);
  	file << to_string(fund) + "\n";
  	file.close();
  	cout << "user " << username << " created\n";
 }

 void loaduserinfo(string username, Userinfo& user) {
 	cout << "loading user account detial...\n";
 	user.name = username;
 	ifstream file;
 	file.open("./usersinfo/" + username + "_account_detial");
 	if (!file.is_open()) {
		cout << "file did not open.\n";
	}
 	string entry, nshares;
 	getline(file, entry,'\n');
 	user.fund = stod(entry);
 	while (file.good()) {
 		getline(file, entry,',');
 		getline(file, nshares,'\n');
 		if (entry.size() != 0 && nshares.size() != 0) {
 			user.accountdetial[entry] = stoi(nshares);
 		}
 	}
 	file.close();
 	return;
 }

 void saveuserinfo(Userinfo& user) {
	ofstream file;
	file.open("./usersinfo/" + user.name + "_account_detial");
	if (!file.is_open()) {
		cout << "file did not open.\n";
	}
	file << to_string(user.fund) << "\n";
	for (auto it = user.accountdetial.begin(); it != user.accountdetial.end(); ++it) {
		file << it->first << "," << it->second << "\n";
	}
	file.close();
 }

 void accountstatus(Userinfo& user) {
 	cout << "\naccount status:\n" << "username: " << user.name  << "\nfund: " << to_string(user.fund) << "\nHolding:\n";
 	for (auto it = user.accountdetial.begin(); it != user.accountdetial.end(); ++it) {
		cout << it->first << "," << it->second << "\n";
	}
	cout << endl;
 }

 void logintoaccount(string username) {
 	cout << "logging into account...\n";
 	ifstream file ("./usersinfo/usersinfo");
	string entry, password;
	int count = 0;
	while (file.good()) 
	{
		getline(file, entry,'[');
		getline(file, entry,'\n');
		getline(file, entry,'\n');
		if (!username.compare(entry)) { // find the username
			getline(file, entry,'\n'); // get stored password
			while (1) { // enter password
				cout << "Please enter password: ";
				cin >> password;
				if (count == 3) return; // exit on the 3rd wrong password
				if (password.compare(entry)) {
					++count;
					cout << "Password does not match...\n";
				} else {
					break; // pass
				}
			}
		}
	}
	file.close();
	Userinfo user;
	loaduserinfo(username, user); // load user info
	while (1) {
		cout << "menu:\nEnter 1 to check account status\nEnter 2 to trade\nEnter 0 to return to main menu\n";
		cout << "Please enter: ";
		int key;
		cin >> key;
		switch (key) {
			case 1:
			accountstatus(user);
			break;
			case 2:
			trade(user);
			break;
			case 0:
			//saveuserinfo(user);
			return;
			default:
			cout << "Error input, please re-enter\n";
		}
		saveuserinfo(user);
	}
	
 }


int main ()
{
	while (1) {
		// main menu
		cout << "menu:\nEnter 1 to creat account\nEnter 2 to login to exsited account\nEnter 0 to exit this simulator\n";
		cout << "Please enter: ";
		int key;
		cin >> key;
		string username, password, password0;
		double fund;
		switch (key) {
			case 1:// create account
			cout << "creating account...\n";
			while (1) { //enter user name
				cout << "Please enter username: ";
				cin >> username;
				if (checkusername(username)) { // check if exist
					cout << "username already exists, please try another one\n";
				} else {
					break;
				}
			}
			while (1) { // enter password
				cout << "Please enter password: ";
				cin >> password;
				cout << "Please re-enter password: ";
				cin >> password0;
				if (password.compare(password0)) {
					cout << "Password does not match...\n";
				} else {
					break;
				}
			}
			//enter initial fund
			cout << "Please enter fund: ";
			cin >> fund;
			creataccount(username, password, fund);// setup new account
			break;
			case 2: // login 
			while (1) {
				cout << "Please enter username: ";
				cin >> username;
				if (!checkusername(username)) {
					cout << "username does not exit, please try another one\n";
				} else {
					break;
				}
			}
			logintoaccount(username);
			break;
			case 0:
			return 0;
			default:
			cout << "Error input, please re-enter\n";
		}
	}

	
	return 0;	
}
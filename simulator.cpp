#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "curl/curl.h"
#include <ctime>

using namespace std;

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

void trade() {
	bool process = false;
	string buyorsell, symb;
	int nshares;
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
		char confirm;
		cin >> confirm;
		if (confirm == 'Y') {
			process = true;
		} else {
			cout << endl << "Please re-enter..." << endl;
		}
		
	}
	cout << "Processing..." << endl;
	float price = stod(getonlinedata(symb));
	time_t now;
		tm *gmtm;
		now = time(0);
		gmtm = gmtime(&now);
	int side = buyorsell.compare("buy") ? -1 : 1;
	string tradelog = to_string(gmtm->tm_hour - 4) + ":" + to_string(gmtm->tm_min) + ":" + to_string(gmtm->tm_sec);
	tradelog += "," + (symb) + "," + to_string(side) + "," + to_string(nshares) + "," + to_string(price) + "\n";
	cout << "\n" << buyorsell << "ing " << nshares << " share(s) of " << (symb)  << " at $" << price << ".\n\n";
	cout << "\ntrade log: " << tradelog << endl;
	//return tradelog;
}

bool checkusername(string username) {
	ifstream file ("./user/usersinfo");
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
  	file.open("./user/usersinfo", std::ios_base::app);
  	file << "[\n" + username + "\n" + password + "\n" + to_string(fund) + "\n]\n";
  	cout << "user " << username << "created\n";
  	file.close();
 }

 void logintoaccount(string username) {
 	cout << "logging into account...\n";
 	ifstream file ("./user/usersinfo");
	string entry, password;
	double fund;
	int count = 0;
	while (file.good()) 
	{
		getline(file, entry,'[');
		getline(file, entry,'\n');
		getline(file, entry,'\n');
		if (!username.compare(entry)) {
			getline(file, entry,'\n');
			while (1) {
				cout << "Please enter password: ";
				cin >> password;
				if (count == 3) return;
				if (password.compare(entry)) {
					++count;
					cout << "Password does not match...\n";
				} else {
					break;
				}
			}
			getline(file, entry,'\n');
			fund = stod(entry);
		}
	}
	file.close();
	while (1) {
		cout << "menu:\nEnter 1 to check account status\nEnter 2 to trade\nEnter 0 to return to main menu\n";
		cout << "Please enter: ";
		int key;
		cin >> key;
		switch (key) {
			case 1:
			cout << "\naccount status:\n" << "username: " << username  << "\nfund: " << fund << "\n\n";
			break;
			case 2:
			trade();
			break;
			case 0:
			return;
			default:
			cout << "Error input, please re-enter\n";
		}
	}
	
 }


int main ()
{
	string log;
	//log = trade();

	while (1) {
		cout << "menu:\nEnter 1 to creat account\nEnter 2 to login to exsited account\nEnter 0 to exit this simulator\n";
		cout << "Please enter: ";
		int key;
		cin >> key;
		string username, password, password0;
		double fund;
		switch (key) {
			case 1:
			cout << "creating account...\n";
			while (1) {
				cout << "Please enter username: ";
				cin >> username;
				if (checkusername(username)) {
					cout << "username already exits, please try another one\n";
				} else {
					break;
				}
			}
			

			while (1) {
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
			cout << "Please enter fund: ";
			cin >> fund;
			creataccount(username, password, fund);
			break;
			case 2:
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
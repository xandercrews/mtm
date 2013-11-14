#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;


string & ltrim(string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
					std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

string & rtrim(string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
					std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

string & trim(string &s) {
	return ltrim(rtrim(s));
}

vector<string> & split(string const & s, char delim,
		vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> split(string const & s, char delim) {
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}

void parse_exec_hosts(string const &exec_hosts,
		vector<string> & exec_host_vector) {

	vector<string> tmp_list;
	split(exec_hosts, '+', tmp_list);

	for (size_t i = 0; i < tmp_list.size(); i++) {
		string tmp_host = tmp_list[i];
		size_t pos = tmp_host.find("/");

		if (pos != string::npos)
			tmp_host.erase(pos);

		exec_host_vector.push_back(tmp_host);
	}
}

// I am completely stumped. netutil.cpp compiles and produces a .o, but I get
// an unresolved linker error. It's like the .o isn't making it into the
// .a. Temp kludge to fix it -- #include the .cpp here. TAKE THIS OUT!
#include "netutil.cpp"

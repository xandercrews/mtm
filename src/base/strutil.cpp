#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>

#if 0
std::string & ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
					std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

std::string & rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
					std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

std::string & trim(std::string &s) {
	return ltrim(rtrim(s));
}

#endif

std::vector<std::string> & split(std::string const & s, char delim,
		std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(std::string const & s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void parse_exec_hosts(std::string const &exec_hosts,
		std::vector<std::string> &exec_host_vector) {

	std::vector<std::string> tmp_list;
	split(exec_hosts, '+', tmp_list);

	for (size_t i = 0; i < tmp_list.size(); i++) {
		std::string tmp_host = tmp_list[i];
		size_t pos = tmp_host.find("/");

		if (pos != std::string::npos)
			tmp_host.erase(pos);

		exec_host_vector.push_back(tmp_host);
	}
}

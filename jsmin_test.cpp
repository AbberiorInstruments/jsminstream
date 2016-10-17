#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "jsmin.hpp"

int main()
{
	std::ifstream ifsb("before.js");
	std::ifstream ifsa("after.js");

	ijsminstream ijsmins(ifsb.rdbuf());
	
	std::stringstream bufb;
	bufb << ijsmins.rdbuf();
	std::string s_b(bufb.str());

	std::stringstream bufa;
	bufa << ifsa.rdbuf();
	std::string s_a(bufb.str());

	if (s_b == s_a)
	{
		std::cout << "Success." << std::endl;
	}
	else
	{
		std::cout << "Error." << std::endl;
	}
}
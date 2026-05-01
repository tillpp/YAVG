#include "Source.hpp"
#include <algorithm>
#include <iostream>
#include <regex>

Source::Source(std::string origin,std::string content):origin(origin){
	lineStart.push_back(0);
	for (size_t i = 0; i < content.size(); i++){
		if(content[i] == '\n'){
			lineStart.push_back(i+1);
		}
	}
}
Source::Position Source::getline(size_t i){
	auto it = std::upper_bound(lineStart.begin(),lineStart.end(),i);
	if(it != lineStart.begin()){
		it = std::prev(it);
	}
	Position position;
	position.line = std::distance(lineStart.begin(),it);
	position.column = i-(*it);
	position.lineStartIndex = *it;
	return position;
}

void Source::error(size_t i,size_t length,std::string msg){
	std::cerr << "\033[31merror:\033[0m "<<msg <<std::endl;
	printCodeSection(i,length,msg);
}
void Source::warning(size_t i,size_t length,std::string msg){
	std::cerr << "\033[33mwarning:\033[0m "<<msg <<std::endl;
	printCodeSection(i,length,msg);
}
void Source::hint(size_t i,size_t length,std::string msg){
	std::cerr << "\033[32mhint:\033[0m "<<msg <<std::endl;
	printCodeSection(i,length,msg);
}
void Source::ref(size_t i,size_t length,std::string msg){
	std::cerr << "\033[34mref:\033[0m "<<msg <<std::endl;
	printCodeSection(i,length,msg);
}

void Source::printCodeSection(size_t i,size_t length,std::string msg){
	auto pos = getline(i);
	std::cerr << "  --> "<< origin<<":"<<(pos.line+1)<<":"<<(pos.column+1)<<std::endl;
	std::string lineCount = std::to_string(pos.line+1);
	size_t lineEnd = content.find_first_of('\n',pos.lineStartIndex);
	
	std::string line;
	if(content.size()>pos.lineStartIndex)
	 	line = content.substr(pos.lineStartIndex,lineEnd-pos.lineStartIndex);
	line = std::regex_replace(line,std::regex("\t")," ");

	std::string whitespace = std::string(lineCount.length(),' ');
	std::cerr << whitespace << " |"<<std::endl;
	std::cerr << lineCount  << " |"<< line<<std::endl;
	std::cerr << whitespace << " |"<< std::string(i-pos.lineStartIndex,' ')<<std::string(length,'^')<<std::endl;
}
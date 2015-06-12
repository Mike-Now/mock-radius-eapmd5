#include "csv_reader.h"
#include "catch.hpp"
#include <map>
#include <string>
namespace radius{
namespace{
	
std::map<std::string,std::string> csvData = {{"Micha�","5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8"},
{"Basia","5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8"}
,{"Ry�","5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8"}};
}
TEST_CASE("Simple read file test", "[readCsvFile]") { 
std::map<std::string,std::string> csvRead = readCsvFile("src/test/csv.csv");
REQUIRE(csvRead.find("Basia") != csvRead.end());
REQUIRE(csvData["Basia"] == csvRead["Basia"]);
 }
 TEST_CASE("File doesn't exists", "[readCsvFile]") { 
std::map<std::string,std::string> csvRead = readCsvFile("k.csv");
REQUIRE(csvRead.empty() == true );
 }
}//namespace

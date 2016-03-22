#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <vector>
#include <string>
using namespace std;

#ifndef SIMPLE_XML_UTILITY
#define SIMPLE_XML_UTILITY

int is_whitespace(char c);

int compare_lowercase(string str1, string str2);

//Split an XML file up into tag content and text content
int parse_xml_string(vector<string>& result, string xml_string);

//Take a list of xml tokens and break it up into top-level items
// + Returns false on syntax error
int create_top_level_blocks(vector<size_t>& result_start, vector<size_t>& result_end, vector<string>* content_string_list, size_t start_index, size_t end_index);

int create_top_level_blocks(vector<size_t>& result_start, vector<size_t>& result_end, vector<string>* content_string_list);

//Returns the tag name of a tag string
int get_tag_info(string& tag_name, int& tag_type, string tag_string);

#endif


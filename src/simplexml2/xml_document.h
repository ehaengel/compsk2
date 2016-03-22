#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <string>
using namespace std;

#include "xml_utility.h"
#include "xml_tag_attribute.h"
#include "xml_content.h"

#ifndef SIMPLE_XML_DOCUMENT
#define SIMPLE_XML_DOCUMENT

class XML_Document
{
public:
	XML_Document();
	~XML_Document();

	/////////////////////
	// Data management //
	/////////////////////

	//File i/o
	int LoadFromFile(string filename);
	int SaveToFile(string filename);

	//Content management
	size_t GetContentCount();
	XML_Content* GetContent(size_t index);

	int AppendContent(XML_Content* xml_content);
	int InsertContent(XML_Content* xml_content, size_t index);

	XML_Content* AppendOpenTag(string tag_name);
	XML_Content* AppendOpenClosedTag(string tag_name);
	XML_Content* AppendText(string text_content);

	//Memory management
	int FreeData();

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();
	int WriteDOTFile(string filename);

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	int ReadFileData(string& result, FILE* handle);

	int CreateHeadContent(vector<string>* content_string_list, vector<size_t> content_start_list, vector<size_t> content_end_list);

	///////////////////////
	// Internal use data //
	///////////////////////

	//XML Document definition
	vector<XML_Content*> head_content_list;
};

#endif

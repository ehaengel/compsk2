#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <string>
#include <vector>
using namespace std;

#include "xml_utility.h"
#include "xml_tag_attribute.h"

#ifndef SIMPLE_XML_CONTENT
#define SIMPLE_XML_CONTENT

class XML_Content
{
public:
	XML_Content();
	~XML_Content();

	///////////////////////////
	// XML Content functions //
	///////////////////////////

	//File i/o
	int SaveToFile(FILE* handle, int depth);

	//Parse for file loading
	int ParseContent(vector<string>* content_string_list, size_t start_index, size_t end_index);

	//Get content data
	int GetType();

	size_t GetChildCount();
	XML_Content* GetChild(size_t index);

	//Content types
	enum
	{
		UNDEFINED_CONTENT = 0,
		TAG_CONTENT = 1,
		TEXT_CONTENT
	};

	//Set content data
	int SetType(int content_type);

	//Append content data
	int AppendChild(XML_Content* xml_content);
	int InsertChild(XML_Content* xml_content, size_t index);

	/////////////////////
	// XML Tag content //
	/////////////////////

	//Get tag data
	int GetTagType();
	string GetTagName();

	size_t GetAttributeCount();
	XML_TagAttribute* GetAttribute(size_t index);

	string GetAttributeValue(string attribute_name);

	enum
	{
		UNDEFINED_TAG_TYPE = 0,
		OPEN_TAG = 1,
		CLOSED_TAG = 2,
		OPEN_CLOSED_TAG = 3
	};

	//Set tag data
	int SetTagType(int tag_type);
	int SetTagName(string tag_name);

	int AppendAttribute(string attribute_name, int attribute_value);
	int AppendAttribute(string attribute_name, double attribute_value);
	int AppendAttribute(string attribute_name, string attribute_value);

	//Append tag data
	XML_Content* AppendOpenTag(string tag_name);
	XML_Content* AppendOpenClosedTag(string tag_name);
	XML_Content* AppendText(string text);

	//////////////////////
	// XML Text content //
	//////////////////////

	string GetText();
	int SetText(string text_content);

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();
	int WriteDOTFile(FILE* handle);

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	int LoadTagAttributes(string tag_string);

	int LoadInternalData(vector<string>* content_string_list, size_t start_index, size_t end_index);

	int FreeData();

	///////////////////////
	// Internal use data //
	///////////////////////

	//XML Content definition
	int content_type;

	vector<XML_Content*> child_content_list;

	//Tag content
	int tag_type;
	string tag_name;
	vector<XML_TagAttribute*> tag_attribute_list;

	//Text content
	string text_content;
};

#endif

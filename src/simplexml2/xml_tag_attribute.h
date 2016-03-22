#include <stdlib.h>
#include <stdio.h>

#include <string>
using namespace std;

#include "xml_utility.h"

#ifndef SIMPLE_XML_TAG_ATTRIBUTE_H
#define SIMPLE_XML_TAG_ATTRIBUTE_H

class XML_TagAttribute
{
public:
	XML_TagAttribute();
	~XML_TagAttribute();

	/////////////////////
	// Data management //
	/////////////////////

	string GetName();
	int SetName(string name);

	string GetValue();
	int SetValue(string value);

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

private:
	//XML Tag Attribute definition
	string name;
	string value;
};

#endif

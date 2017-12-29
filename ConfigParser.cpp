/**
 * @file ConfigParser.cpp
 * @brief Config file reading service for SorterHunter
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 *
 * Copyright (c) 2017 Bert Dobbelaere
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ConfigParser.h"
#include <map>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cctype>

using std::string;

typedef std::map<string, u32> U32Map; ///< key/value pairs for integer parameters
typedef std::map<string,Network_t> NetworkMap; ///< key/value pairs for network parameters

/**
 * Config parser internal kitchen class
 */
class ConfigParser::Data{
	public:
		void clear();
		bool addKeyValue(string key, string value, u32 linenr);
		bool verifyNumKey(string key,u32 minval,u32 maxval) const;
		/* Data members (public) */
		U32Map u32map; ///< key/value pairs for integer parameters
		NetworkMap networkmap; ///< key/value pairs for network parameters
	private:
		bool addKeyNetworkValue(string key, string value, u32 linenr); 
};

/**
 * Remove left and right whitespace + commented text.
 * @param l text to be stripped
 * @return stripped text
 */
static string stripline(string l)
{
	size_t pos=l.find('#');
	if(pos!=string::npos)
		l=l.substr(0,pos);
	pos=l.find_last_not_of(" \t\n\r");
	if(pos!=string::npos)
		l=l.substr(0,pos+1);
	else
		l="";
	pos=l.find_first_not_of(" \t\n\r");
	if(pos!=string::npos)
		l=l.substr(pos);
	else
		l="";
	return l;
}

/**
 * Convert decimal string to integer
 * @param s Input string
 * @param result Converted integer
 * @return true on success
 */
static bool value2u32(string s, u32 &result)
{
	char dummy;
	return sscanf(s.c_str(),"%u%c",&result,&dummy)==1u;
}


/**
 * Forget all parameters in database
 */
void ConfigParser::Data::clear()
{
	u32map.clear();
	networkmap.clear();
}

/**
 * Process a (key,value) pair of strings from the config file
 * @param key LHS expression
 * @param value RHS expression
 * @param linenr Line number in config file
 * @return true on success
 */

bool ConfigParser::Data::addKeyValue(string key, string value, u32 linenr)
{
	if((key=="FixedPrefix")||(key=="InitialNetwork"))
	{
		/* For these two keys, delegate further processing to network value handler */
		return addKeyNetworkValue(key,value,linenr);
	}
	
	if(u32map.find(key)!=u32map.end())
	{
		printf("Duplicate key '%s' in config file, line %u\n",key.c_str(),linenr);
		return false;
	}
	
	u32 numval;
	bool ok=value2u32(value,numval);
	if(!ok)
	{
		printf("Numeric rvalue expected in config file, line %u\n",linenr);
		return false;
	}
	else
	{
		u32map.insert(std::pair<string,u32>(key,numval));
		return true;
	}
}

/**
 * Process a (key,value) pair of strings from the config file if a network value is expected
 * @param key LHS expression
 * @param value RHS expression (list of pairs)
 * @param linenr Line number in config file
 * @return true on success
 */
bool ConfigParser::Data::addKeyNetworkValue(string key, string value, u32 linenr)
{
	if(networkmap.find(key)!=networkmap.end())
	{
		printf("Duplicate key '%s' in config file, line %u\n",key.c_str(),linenr);
		return false;
	}
	
	Network_t network;
	u32 state=0; // 0: scan for '(' / 1: scan for inner ',' / 2: scan for ')' / 3: scan for outer ',' / 100: error
	size_t tokenstart;
	u32 p1,p2;
	for(size_t idx=0;idx<value.size();idx++)
	{
		char c=value[idx];
		switch(state)
		{
			case 0:
				if(c=='(')
				{
					tokenstart=idx+1;
					state=1;
				}
				else if(!isspace(c))
				{
					state=100;
				}
				break;
			case 1:
				if(c==',')
				{
					bool ok=value2u32(stripline(value.substr(tokenstart,idx-tokenstart)),p1);
					state=ok ? 2: 100;
					tokenstart=idx+1;
				}
				break;
			case 2:
				if(c==')')
				{
					bool ok=value2u32(stripline(value.substr(tokenstart,idx-tokenstart)),p2);
					state=ok ? 3: 100;
					if(ok && (p1<=255) && (p2<=255))
					{
						Pair_t p={(u8)p1,(u8)p2};
						network.push_back(p);
					}
				}
				break;
			case 3:
				if(c==',')
				{
					state=0;
				}
				else if(!isspace(c))
				{
					state=100;
				}
				break;
			default:
				break;
		}
	}
	
	if((state!=0) && (state!=3))
	{
		printf("Config file parse error line %u\n",linenr);
		return false;
	}
	else
	{
		networkmap.insert(std::pair<string,Network_t>(key,network));
		return true;
	}
}

/**
 * Verify presence and range of key with expected numeric value
 * @param key Parameter name
 * @param minval Minimum expected value (inclusive)
 * @param maxval Maximum expected value (inclusive)
 * @return true If present and within range
 */
bool ConfigParser::Data::verifyNumKey(string key,u32 minval,u32 maxval) const
{
	U32Map::const_iterator it=u32map.find(key);
	if(it==u32map.end())
	{
		printf("Missing mandatory key '%s' in config file.\n",key.c_str());
		return false;
	}
	u32 val= it->second;
	if((val<minval)||(val>maxval))
	{
		printf("Value for key '%s' should be in range %u..%u (was %u)\n",key.c_str(),minval, maxval, val);
		return false;
	}
	return true;
}

bool ConfigParser::parseConfig(const char *filename)
{
	data->clear();
	std::ifstream infile(filename);
	if(!infile)
	{
		perror("Could not open config file");
		return false;
	}
	
	bool fileok=true;
	string line;
	u32 linenr=1;
 
    while(getline(infile, line)) {
        /* Strip */
        line=stripline(line);
        
        if(line.size()>0)
        {
			size_t klen=line.find('=');
			if((klen==string::npos) || (klen<1u))
			{
				printf("Parse error at %s:%u\n",filename,linenr);
				fileok=false;
			}
			else
			{
				string key=stripline(line.substr(0,klen));
				string value=stripline(line.substr(klen+1));
				bool ok=data->addKeyValue(key,value,linenr);
				if(!ok)
					fileok=false;	
			}
		}
        linenr++;
    }
 
    infile.close();
    
    fileok = fileok && data->verifyNumKey("Ninputs",2,NMAX);
    fileok = fileok && data->verifyNumKey("Symmetric",0,1);
	
	return fileok;
}

u32 ConfigParser::getU32(string key, u32 defaultval) const
{
	U32Map::const_iterator it=data->u32map.find(key);
	if(it==data->u32map.end())
	{
		return defaultval;
	}
	else
	{
		return it->second;
	}
}


const Network_t &ConfigParser::getNetwork(string key) const
{
	static Network_t empty_net;
	NetworkMap::const_iterator it=data->networkmap.find(key);
	if(it==data->networkmap.end())
	{
		return empty_net;
	}
	else
	{
		return it->second;
	}
}


ConfigParser::ConfigParser()
{
	data=new Data; // Data default constructor starts with empty databases (ok)
}

ConfigParser::~ConfigParser()
{
	delete data;
}

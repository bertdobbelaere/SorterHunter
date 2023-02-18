/**
 * @file ConfigParser.h 
 * @brief Config file reading service for SorterHunter
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 *
 * Copyright (c) 2022 Bert Dobbelaere
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

#pragma once
#include "htypes.h"
#include <string>

namespace sh {
	/**
	 * Class performing config file processing
	 */

	class ConfigParser
	{
	public:
		/**
		 * Object init
		 */
		ConfigParser();

		/**
		 * Reads a config file into the object structures
		 * @param filename Name of config file. Will be opened for reading only.
		 * @return true if config file was successfully read
		 */
		bool parseConfig(const char* filename);

		/**
		 * Reads an integer parameter from the config file
		 * If the parameter was not specified, the default is used
		 * @param key Parameter name
		 * @param defaultval Default value if parameter was not found
		 * @return Parameter value
		 */
		uint64_t getInt(std::string key, uint64_t defaultval = 0) const;

		/**
		 * Reads a parameter containing sorting network pairs from the config file
		 * An empty network is returned if the parameter was not found
		 * @param key Parameter name
		 * @return Network composed of CEs
		 */
		const Network_t& getNetwork(std::string key) const;

		/**
		 * Clean up
		 */
		~ConfigParser();
	private:
		class Data;
		Data* data;
	};
}
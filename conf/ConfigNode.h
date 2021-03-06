/*
Copyright (C) 2002 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CONFIGNODE_H
#define CONFIGNODE_H

#include <string>
#include "Configuration.h"

class ConfigNode
{
	friend class Configuration;
	ConfigNode(Configuration& config_, std::string key_)
		: config(config_), key(key_)
	{ }

 public:
	~ConfigNode() { }

	// fix "assignment operator could not be generated" warning
	const ConfigNode& operator = (const ConfigNode &other) {
		config = other.config;
		key = other.key;
		return *this;
	}

	std::string get_string(const char *defaultvalue="") {
		std::string s;
		config.value(key, s, defaultvalue);
		return s;
	}
	int get_int(int defaultvalue=0) {
		int i;
		config.value(key, i, defaultvalue);
		return i;
	}
	bool get_bool(bool defaultvalue=false) {
		bool b;
		config.value(key, b, defaultvalue);
		return b;
	}

	void set(std::string value) {
		config.set(key, value);
	}
	void set(const char* value) {
		config.set(key, value);
	}
	void set(int value) {
		config.set(key, value);
	}
	void set(bool value) {
		config.set(key, value);
	}

 private:
	Configuration& config;
	std::string key;

};


#endif

/*  
	Copyright (c) 2012, Alexey Saenko
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/ 

#ifndef UTILS_H_
#define UTILS_H_

#include <sstream>
#include <vector>
#include <vector>
#include <algorithm>

template<class T>
std::string to_string(T v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
}

template <class T>
bool vector_fast_remove(std::vector<T> &v, const T &val) {
	typename std::vector<T>::iterator it = std::find(v.begin(), v.end(), val);
	if(it == v.end())
		return false;
	*it = v.back();
	v.pop_back();
	return true;
}

#endif /* UTILS_H_ */

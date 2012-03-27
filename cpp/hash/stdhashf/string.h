#ifndef HOMEWORKS_STDHASHF_STRING
#define HOMEWORKS_STDHASHF_STRING

#include <string>

namespace Homeworks{

namespace HashF{

	template<>
	inline int hashf<std::string>(const std::string &key, int size){
		return iterableSTLHashf(key, size);
	}

};

};

#endif
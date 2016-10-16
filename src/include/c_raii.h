#include <cstdio>
#include <string>
#include <system_error>

namespace hexgame {

// Simple RAII wrapper for C lib file object.
class CFile {
public:
	CFile(const std::string file_name, const std::string mode) {
		fp = fopen(file_name.c_str(), mode.c_str());
		if (!fp) throw std::system_error(std::error_code(errno, std::system_category()),
				"Unable to open "+file_name);
	}
	virtual ~CFile() {
		if (fp) fclose(fp);
	}

	FILE *fp = nullptr;
};

}


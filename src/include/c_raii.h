#include <cstdio>
#include <cstdlib>
#include <string>
#include <system_error>
#include <tuple>

namespace hexgame {

struct LineBuffer { // Helper for CFile.
	char *line = nullptr;
	size_t len = 0;

	~LineBuffer() {
		if (line) free(line);
	}
};

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
	std::tuple<ssize_t, std::string> readline() {
		LineBuffer lb;
		int r = getline(&lb.line, &lb.len, fp);
		return std::tuple<ssize_t, std::string>(r, lb.line==nullptr?"":lb.line);
	}

	FILE *fp = nullptr;
};

}


#pragma once

#include <string>

namespace hexgame {

class Settings {
public:
	static Settings *i();
	void setAssetDir(std::string assetDir);
	const std::string& getAssetDir() const;

private:
	static Settings instance;

	Settings();
	~Settings();
	std::string assetDir;
};

}


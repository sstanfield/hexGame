#include "settings.h"

using namespace hexgame;

Settings Settings::instance;

Settings *Settings::i() {
	return &instance;
}

void Settings::setAssetDir(std::string assetDir) {
	this->assetDir = assetDir;
}

const std::string &Settings::getAssetDir() const {
	return assetDir;
}

void Settings::setShaderDir(std::string shaderDir) {
	this->shaderDir = shaderDir;
}

const std::string &Settings::getShaderDir() const {
	return shaderDir;
}

Settings::Settings() {
}

Settings::~Settings() {
}


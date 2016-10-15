#pragma once

#include <map>

namespace hexgame { namespace render {

class GLCapabilities {
	public:
		static GLCapabilities *i();

		bool setCap(unsigned int cap, bool state);
		bool enable(unsigned int cap);
		bool disable(unsigned int cap);
		bool clearErrors();
		void getState(std::map<unsigned int, bool> &state);
		void setState(std::map<unsigned int, bool> &state);

	private:
		GLCapabilities();

		std::map<unsigned int, bool> caps;
		static GLCapabilities instance;
};

} }


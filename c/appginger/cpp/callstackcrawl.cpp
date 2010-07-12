#include "callstackcrawl.hpp"


Ref * CallStackCrawl::next() {
	Ref * answer = this->sp;
	this->sp = ToRefRef( sp[ SP_PREV_SP ] );
	return answer;
}

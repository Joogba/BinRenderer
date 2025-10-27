#include "Logger.h"

using namespace std;

namespace BinRenderer
{
	// 정적 멤버 초기화
	unique_ptr<Logger> Logger::instance = nullptr;
}
#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <cassert>
#include <string>
#include <format>

namespace BinRenderer
{
    using namespace std;

    /**
     * @brief 싱글톤 Logger 클래스
     * 
     * 콘솔과 파일에 동시에 로그를 기록합니다.
     * Vulkan과 독립적이며 전체 프로젝트에서 사용 가능합니다.
     */
    class Logger
    {
    private:
        static unique_ptr<Logger> instance;

     ofstream logFile;
        size_t messagesProcessed;

        // 싱글톤 생성자
        Logger() : messagesProcessed(0)
        {
  logFile.open("log.txt", ios::out | ios::trunc);

      if (!logFile.is_open()) {
           cerr << "ERROR: Could not open log.txt for writing!" << endl;
   }
        }

  // 복사 생성자 삭제
  Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        template <typename T>
    void formatToStream(ostringstream& oss, T&& arg)
   {
    oss << forward<T>(arg);
        }

        template <typename T, typename... Args>
   void formatToStream(ostringstream& oss, T&& first, Args&&... args)
        {
       oss << forward<T>(first);
       if (sizeof...(args) > 0) {
                oss << " ";
     formatToStream(oss, forward<Args>(args)...);
            }
   }

    public:
        ~Logger()
        {
        if (logFile.is_open()) {
  logFile.flush();
  logFile.close();
     }
        }

        static Logger& getInstance()
        {
      if (!instance) {
      instance = unique_ptr<Logger>(new Logger());
   }
            return *instance;
     }

        static void printLog(string message)
      {
            auto& logger = getInstance();

         cout << message << endl;

         if (logger.logFile.is_open()) {
   logger.logFile << message << endl;
                logger.logFile.flush(); // 바로 기록
       logger.messagesProcessed++;
            }
            else {
            cerr << "WARNING: Log file is not open, message lost: " << message << endl;
  }
        }

 static size_t getMessagesProcessed()
      {
    return getInstance().messagesProcessed;
        }
    };

    /**
     * @brief 포맷 문자열을 사용한 로그 출력
     * 
     * std::format을 사용하여 타입 안전한 로그를 출력합니다.
     * 
     * @example printLog("Value: {}, Name: {}", 42, "Test");
     */
    template <typename... Args>
    void printLog(std::format_string<Args...> fmt, Args&&... args)
    {
        string message = std::format(fmt, std::forward<Args>(args)...);
        Logger::printLog(message);
    }

    /**
     * @brief 에러 메시지 출력 후 종료
     * 
     * 로그를 출력한 후 assert(false)와 exit()를 호출합니다.
     */
    template <typename... Args>
    void exitWithMessage(std::format_string<Args...> fmt, Args&&... args)
    {
        string message = std::format(fmt, std::forward<Args>(args)...);
        Logger::printLog(message);
        assert(false);
        exit(EXIT_FAILURE);
    }

} // namespace BinRenderer

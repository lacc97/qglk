#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>

namespace Glk {
    class ExitException : public std::exception {
        public:
            explicit ExitException(bool interrupted_ = false) : m_Interrupted(interrupted_) {}

            const char *what() const noexcept override {
                if(interrupted())
                    return "interrupted";
                else
                    return "exit";
            }

            [[nodiscard]] inline bool interrupted() const {
                return m_Interrupted;
            }

        private:
            bool m_Interrupted;
    };
}

#endif

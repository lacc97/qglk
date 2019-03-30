#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>

namespace Glk {
    class ExitException : public std::exception {
        public:
            ExitException(bool interrupted_ = false) : m_Interrupted(interrupted_) {}

            inline bool interrupted() const {
                return m_Interrupted;
            }

        private:
            bool m_Interrupted;
    };
}

#endif

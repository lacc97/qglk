#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include <fmt/format.h>

#include "glk.hpp"

#include "log/pointerwrap.hpp"
#include "thread/taskrequest.hpp"

namespace Glk {
    class Window;

    class EventQueue : public QObject {
        Q_OBJECT
    public:
        static std::string_view typeName(glui32 t);


        explicit EventQueue(QObject* parent = nullptr);
        

        event_t pop();
        event_t popLineEvent(Glk::Window* win);
        event_t poll();
        

        void interrupt();

        [[nodiscard]] inline bool isInterrupted() const {
            return m_Terminate;
        }
        
        [[nodiscard]] inline bool isWaitingOnSemaphore() const {
            return m_Semaphore.available() == 0;
        }

        void requestImmediateSynchronization();
        
    public slots:
        void cleanWindowEvents(winid_t win);
        void push(const event_t& ev);
        void pushTaskEvent(Glk::TaskEvent* ev); // push code that should be executed in glk thread
        void pushTimerEvent();

    signals:
        void canSynchronize();
        
    private:
        QQueue<event_t> m_Queue;
        QQueue<TaskEvent*> m_TaskEventQueue;
        QMutex m_AccessMutex;
        QSemaphore m_Semaphore;
        
        bool m_Terminate;
    };
}

namespace fmt {
    template <>
    struct formatter<event_t> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const event_t& e, FormatContext &ctx) {
            return format_to(ctx.out(), "{} {{{}, {}, {}}}", Glk::EventQueue::typeName(e.type), wrap::ptr(e.win), e.val1, e.val2);
        }
    };
}
std::ostream& operator<<(std::ostream& os, const event_t& e);

#endif

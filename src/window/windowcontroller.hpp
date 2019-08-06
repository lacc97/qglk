#ifndef WINDOWCONTROLLER_HPP
#define WINDOWCONTROLLER_HPP

#include <cassert>

#include <atomic>
#include <memory>
#include <set>

#include <QSize>
#include <QWidget>

#include "glk.hpp"

#include "inputprovider.hpp"

namespace Glk {
    class Window;

    class PairWindow;

    class WindowController {

        public:
            static WindowController* createController(glui32 wintype, PairWindow* parent, glui32 rock);

            virtual ~WindowController();


            virtual void cancelCharInput();

            [[nodiscard]] virtual event_t cancelLineInput();

            virtual void cancelMouseInput();

            virtual void closeWindow();

            virtual void requestCharInput(bool unicode);

            virtual void requestLineInput(void* buf, glui32 maxLen, glui32 initLen, bool unicode);

            virtual void requestMouseInput();

            void requestSynchronization();

            [[nodiscard]] inline bool requiresSynchronization() const {
                return m_RequiresSynchronization;
            }

            virtual void synchronize();


            [[nodiscard]] inline bool lineInputEchoes() const {
                return m_LineInputEcho;
            }

            inline void setLineInputEcho(bool echoes) {
                m_LineInputEcho = true;
            }

            [[nodiscard]] inline const std::set<Qt::Key>& lineInputTerminators() const {
                return m_LineInputTerminators;
            }

            inline void setLineInputTerminators(const std::set<Qt::Key>& terminators) {
                m_LineInputTerminators = terminators;
            }

            template<class WidgetT = QWidget>
            [[nodiscard]] inline WidgetT* widget() const {
                static_assert(std::is_base_of_v<QWidget, WidgetT>);
                assert(dynamic_cast<WidgetT*>(mp_Widget.get()));

                return static_cast<WidgetT*>(mp_Widget.get());
            }

            template<class WindowT = Window>
            [[nodiscard]] inline WindowT* window() const {
                static_assert(std::is_base_of_v<Window, WindowT>);
                assert(dynamic_cast<WindowT*>(mp_Window.get()));

                return static_cast<WindowT*>(mp_Window.get());
            }

            [[nodiscard]] virtual QPoint glkPos(const QPoint& qtPos) const = 0;

            [[nodiscard]] virtual QSize glkSize() const = 0;

            [[nodiscard]] virtual QSize toQtSize(const QSize& glk) const = 0;

        protected:
            explicit WindowController(Window* win, QWidget* widg);


            [[nodiscard]] inline CharInputRequest* charInputRequest() const {
                return mp_CharInputRequest.get();
            }

            inline void setCharInputRequest(CharInputRequest* cir) {
                mp_CharInputRequest.reset(cir);
            }

            [[nodiscard]] inline LineInputRequest* lineInputRequest() const {
                return mp_LineInputRequest.get();
            }

            inline void setLineInputRequest(LineInputRequest* lir) {
                mp_LineInputRequest.reset(lir);
            }

            [[nodiscard]] inline MouseInputRequest* mouseInputRequest() const {
                return mp_MouseInputRequest.get();
            }

            inline void setMouseInputRequest(MouseInputRequest* mir) {
                mp_MouseInputRequest.reset(mir);
            }

        private:
            std::unique_ptr<Window> mp_Window;
            std::unique_ptr<QWidget> mp_Widget;
            std::atomic_bool m_RequiresSynchronization;

            bool m_LineInputEcho;
            std::unique_ptr<CharInputRequest> mp_CharInputRequest;
            std::unique_ptr<LineInputRequest> mp_LineInputRequest;
            std::set<Qt::Key> m_LineInputTerminators;
            std::unique_ptr<MouseInputRequest> mp_MouseInputRequest;
    };
}


#endif //WINDOWCONTROLLER_HPP

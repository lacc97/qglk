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


            [[nodiscard]] virtual bool supportsCharInput() const;

            [[nodiscard]] virtual bool supportsHyperlinkInput() const;

            [[nodiscard]] virtual bool supportsLineInput() const;

            [[nodiscard]] virtual bool supportsMouseInput() const;

            virtual void closeWindow();

            void requestSynchronization();

            [[nodiscard]] inline bool requiresSynchronization() const {
                return m_RequiresSynchronization;
            }

            virtual void synchronize();


            [[nodiscard]] inline HyperlinkInputProvider* hyperlinkProvider() const {
                return mp_HyperlinkInputProvider;
            }

            [[nodiscard]] inline KeyboardInputProvider* keyboardProvider() const {
                return mp_KeyboardInputProvider;
            }

            [[nodiscard]] inline MouseInputProvider* mouseProvider() const {
                return mp_MouseInputProvider;
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

        private:
            std::unique_ptr<Window> mp_Window;
            std::unique_ptr<QWidget> mp_Widget;
            std::atomic_bool m_RequiresSynchronization;

            KeyboardInputProvider* mp_KeyboardInputProvider;
            MouseInputProvider* mp_MouseInputProvider;
            HyperlinkInputProvider* mp_HyperlinkInputProvider;
    };
}


#endif //WINDOWCONTROLLER_HPP

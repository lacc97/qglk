#ifndef PAIRWINDOW_HPP
#define PAIRWINDOW_HPP

#include <QLayout>

#include "constraint.hpp"
#include "window.hpp"

namespace Glk {
    class PairWindow : public Window {
            Q_OBJECT

            friend class WindowConstraint;
        public:
            PairWindow(Window* key_, Window* first_, Window* second_, WindowConstraint* constraint_);
            ~PairWindow();

            void removeChildWindow(Window* ptr);

            void setKeyWindow(Window* keywin) { // a call to setKeyWindow should be followed by a call to setConstraint(constraint())
                mp_Key = keywin;
            }
            inline Window* keyWindow() const {
                return mp_Key;
            }
            inline Window* firstWindow() const {
                return mp_First;
            }
            inline Window* secondWindow() const {
                return mp_Second;
            }
            inline WindowConstraint* constraint() const {
                return mp_Constraint;
            }
            inline void setConstraint(WindowConstraint* c) {
                if(c != mp_Constraint) {
                    delete mp_Constraint;
                    mp_Constraint = c;
                }

                mp_Constraint->setupWindows(this, mp_Key, mp_First, mp_Second);
            }

            bool isDescendant(Glk::Window* win) const;

            Glk::Window::Type windowType() const override {
                return Window::Pair;
            }

            QSize windowSize() const override {
                return QSize();
            }

            void clearWindow() override {
            }

        protected:
            QSize pixelsToUnits(const QSize& pixels) const override;
            QSize unitsToPixels(const QSize& units) const override;

        private:
            Window* mp_Key;
            Window* mp_First; // mp_First is either the key window or a parent of the key window
            Window* mp_Second;
            WindowConstraint* mp_Constraint;
    };
}

#endif

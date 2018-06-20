#ifndef PAIRWINDOW_HPP
#define PAIRWINDOW_HPP

#include <QLayout>

#include "constraint.hpp"
#include "window.hpp"

namespace Glk {
//     class PairLayout : public QLayout {
//         public:
//             PairLayout(QWidget* parent_, WindowConstraint* constraint_);
//             ~PairLayout();
//
//             void addItem(QLayoutItem* it) override;
//             int count() const override;
//             QLayoutItem* itemAt(int index) const override;
//             QSize minimumSize() const override;
//             void setGeometry(const QRect& rect) override;
//             QSize sizeHint() const override;
//             QLayoutItem* takeAt(int index) override;
//
//             inline WindowConstraint* constraint() const {
//                 return mp_Constraint;
//             }
//             inline void setConstraint(WindowConstraint* c) {
//                 delete mp_Constraint;
//                 mp_Constraint = c;
//             }
//
//         private:
//             WindowConstraint* mp_Constraint;
//             QLayoutItem* mp_Key;
//             QLayoutItem* mp_Split;
//     };

    class PairWindow : public Window {
        friend class WindowConstraint;
        public:
            PairWindow(Window* key_, Window* split_, WindowConstraint* constraint_);
            ~PairWindow();

            void removeChildWindow(Window* ptr);

            inline void swapWindows() {
                Window* tmp = mp_Key;
                mp_Key = mp_Split;
                mp_Split = tmp;
            }
            inline Window* keyWindow() const {
                return mp_Key;
            }
            inline Window* splitWindow() const {
                return mp_Split;
            }
            inline WindowConstraint* constraint() const {
                return mp_Constraint;
            }
            inline void setConstraint(WindowConstraint* c) {
                delete mp_Constraint;
                mp_Constraint = c;

                mp_Constraint->setupWindows(this, mp_Key, mp_Split);
            }

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
            Window* mp_Split;
            WindowConstraint* mp_Constraint;
    };
}

#endif

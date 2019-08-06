#ifndef PAIRWINDOW_HPP
#define PAIRWINDOW_HPP

#include <memory>

#include "constraint.hpp"
#include "window.hpp"
#include "pairwindowcontroller.hpp"

namespace Glk {
    class PairWindow : public Window {
            friend class WindowArrangement;

        public:
            PairWindow(Window* key, Window* first, Window* second, WindowArrangement* winArrangement,
                       PairWindowController* winController, PairWindow* parent);

            ~PairWindow() final = default;


            void clearWindow() override {}

            [[nodiscard]] Glk::Window::Type windowType() const override {
                return Window::Pair;
            }


            [[nodiscard]] bool isDescendant(Glk::Window* win) const;

            // Returns reamining sibling
            Glk::Window* removeChild(Window* deadChild);

            void replaceChild(Window* oldChild, Window* newChild);

            void setArrangement(Window* key, WindowArrangement* arrange);


            [[nodiscard]] inline WindowArrangement* arrangement() const {
                return mp_Arrangement.get();
            }

            [[nodiscard]] inline Window* keyWindow() const {
                return mp_Key;
            }

            [[nodiscard]] inline Window* firstWindow() const {
                return mp_First;
            }

            [[nodiscard]] inline Window* secondWindow() const {
                return mp_Second;
            }

        private:
            Window* mp_Key;
            Window* mp_First;
            Window* mp_Second;
            std::unique_ptr<WindowArrangement> mp_Arrangement;
    };
}

#endif

#ifndef TEXTGRIDWINDOW_HPP
#define TEXTGRIDWINDOW_HPP

#include <vector>

#include <QIODevice>
#include <QPoint>

#include "textgridwindowcontroller.hpp"
#include "window.hpp"
#include "window_stream_driver.hpp"

namespace qglk::stream_drivers {
  class text_grid;
}

namespace Glk {
    class TextGridWindow : public Window {
        public:
            TextGridWindow(TextGridWindowController* winController, PairWindow* winParent, glui32 winRock);

            ~TextGridWindow() final = default;


            void clearWindow() override;

            void moveCursor(glui32 x, glui32 y) override;


            [[nodiscard]] inline const std::vector<std::vector<glui32>>& grid() const {
                return m_CharArray;
            }

            inline void setGridCursor(glui32 xpos, glui32 ypos) {
                m_Cursor = QPoint(xpos, ypos);
            }

            [[nodiscard]] inline QSize gridSize() const {
                return m_GridSize;
            }

            void resizeGrid(QSize newSize);

            bool writeChar(glui32 ch);

        private:
            static constexpr glui32 EMPTY_CHAR = ' ';

            std::vector<std::vector<glui32>> m_CharArray;
            QSize m_GridSize;
            QPoint m_Cursor;
    };
}

namespace qglk::stream_drivers {
  class text_grid : public window {
   public:
    explicit text_grid(Glk::TextGridWindow* win) : window{win} {}

   protected:
    auto xsputn(const char_type* s, std::streamsize count) -> std::streamsize final;
  };
}

#endif

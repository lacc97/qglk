#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

#include <list>
#include <memory>
#include <vector>

#include <QString>

#include "stylemanager.hpp"
#include "textbufferwindowcontroller.hpp"
#include "window.hpp"
#include "window_stream_driver.hpp"

namespace qglk::stream_drivers {
  class text_buffer;
}

namespace Glk {
    class TextBufferWindow : public Window {
            friend class ::qglk::stream_drivers::text_buffer;

        public:
            TextBufferWindow(TextBufferWindowController* winController, PairWindow* winParent, glui32 winRock);

            ~TextBufferWindow() final = default;


            void clearWindow() override;

            bool drawImage(glui32 img, glsi32 param1, glsi32 param2, QSize size) override;

            void flowBreak() override;

            void pushHyperlink(glui32 linkValue) override;

            void pushStyle(Glk::Style::Type style) override;


            void writeString(QString str);


            [[nodiscard]] inline const StyleManager& styles() const {
                return m_Styles;
            }

        private:
            StyleManager m_Styles;
    };
}

namespace qglk::stream_drivers {
  class text_buffer : public window {
   public:
    explicit text_buffer(Glk::TextBufferWindow* win) : window{win} {}

   protected:
    auto xsputn(const char_type* s, std::streamsize count) -> std::streamsize final;
  };
}

#endif

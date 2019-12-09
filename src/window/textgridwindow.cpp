#include "textgridwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QFontDatabase>
#include <QPainter>
#include <QResizeEvent>

#include "qglk.hpp"

Glk::TextGridDevice::TextGridDevice(Glk::TextGridWindow* win)
    : WindowDevice{win} {}

qint64 Glk::TextGridDevice::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 Glk::TextGridDevice::writeData(const char* data, qint64 len) {
    assert(len % 4 == 0);

    auto buf = reinterpret_cast<const glui32*>(data);
    qint64 bufLen = len / 4;

    qint64 writeCount;

    for(writeCount = 0; writeCount < bufLen; writeCount++) {
        if(!window<TextGridWindow>()->writeChar(buf[writeCount]))
            break;
    }

    return writeCount * 4;
}

Glk::TextGridWindow::TextGridWindow(Glk::TextGridWindowController* winController, Glk::PairWindow* winParent,
                                    glui32 winRock)
    : Window(Type::TextGrid, winController, new TextGridDevice{this}, winParent, winRock),
      m_CharArray{{EMPTY_CHAR}},
      m_GridSize{1, 1},
      m_Cursor(0, 0) {}

void Glk::TextGridWindow::clearWindow() {
    std::for_each(m_CharArray.begin(), m_CharArray.end(), [](auto& column) {
        std::fill(column.begin(), column.end(), EMPTY_CHAR);
    });

    m_Cursor = {0, 0};

    controller()->requestSynchronization();
}

void Glk::TextGridWindow::moveCursor(glui32 x, glui32 y) {
    m_Cursor = {std::min<int>(m_GridSize.width() - 1, x), std::min<int>(m_GridSize.height() - 1, y)};
}

bool Glk::TextGridWindow::writeChar(glui32 ch) {
    QSize ws(m_CharArray.size(), m_CharArray[0].size());

    if(m_Cursor.x() < 0 || m_Cursor.y() < 0 || m_Cursor.x() >= ws.width() || m_Cursor.y() >= ws.height())
        return false;

    controller()->requestSynchronization();

    if(ch == '\n') {
        m_Cursor = QPoint(0, m_Cursor.y() + 1);
        return true;
    }

    m_CharArray[m_Cursor.x()][m_Cursor.y()] = ch;
    m_Cursor += QPoint(1, 0);

    if(m_Cursor.x() == ws.width()) {
        m_Cursor.setX(0);
        m_Cursor += QPoint(0, 1);
    }

    return true;
}

void Glk::TextGridWindow::resizeGrid(QSize newSize) {
    QSize oldSize = m_GridSize;

    m_CharArray.resize(std::max(1, newSize.width()));

    std::for_each(m_CharArray.begin(), m_CharArray.end(), [newSize](auto& column) {
        column.resize(std::max(1, newSize.height()));
    });

    for(int xx = oldSize.width(); xx < newSize.width(); xx++) {
        std::for_each(m_CharArray[xx].begin(), m_CharArray[xx].end(), [](auto& ch) {
            ch = EMPTY_CHAR;
        });
    }

    if(newSize.height() > oldSize.height()) {
        std::for_each(m_CharArray.begin(), m_CharArray.end(), [oldSize, newSize](auto& column) {
            for(int yy = oldSize.height(); yy < newSize.height(); yy++) {
                column[yy] = EMPTY_CHAR;
            }
        });
    }

    m_GridSize = newSize;
}


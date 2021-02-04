#include "object.hpp"

#include "qglk.hpp"

void qglk::object::destroy_base() noexcept {
    QGlk::getMainWindow().dispatch().unregisterObject(this);
}

void qglk::object::init_base() noexcept {
    QGlk::getMainWindow().dispatch().registerObject(this);
}

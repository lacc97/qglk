#include "object.hpp"

#include "qglk.hpp"

void qglk::object::register_object() noexcept {
    QGlk::getMainWindow().dispatch().registerObject(this);
}

void qglk::object::unregister_object() noexcept {
    QGlk::getMainWindow().dispatch().unregisterObject(this);
}

void qglk::object_list::add(qglk::object* p_t) {
    assert(p_t);
    assert(p_t->get_type() == m_type);

    p_t->m_iterator = m_list.emplace(m_list.end(), p_t);
}

qglk::object* qglk::object_list::next_impl(qglk::object* p_cur) const noexcept {
    assert(!p_cur || p_cur->get_type() == m_type);

    if(p_cur) {
        auto next_it = std::next(p_cur->m_iterator);
        if(next_it == m_list.end())
            return nullptr;
        else
            return *next_it;
    } else if(m_list.empty()) {
        return nullptr;
    } else {
        return m_list.front();
    }
}

void qglk::object_list::remove(qglk::object* p_t) {
    assert(p_t);
    assert(p_t->get_type() == m_type);
    assert(!m_list.empty());

    m_list.erase(std::exchange(p_t->m_iterator, {}));
}

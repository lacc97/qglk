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
    assert(p_t->m_index == -1);

    if(!m_free_list.empty()) {
        size_t idx = m_free_list.back();

        assert(m_entries[idx].deleted == true);
        p_t->m_index = idx;
        m_entries[idx].ptr = p_t;
        m_entries[idx].deleted = false;

        m_free_list.pop_back();
    } else {
        p_t->m_index = m_entries.size();
        m_entries.emplace_back(p_t);
    }
}

cppcoro::generator<qglk::object*> qglk::object_list::as_range() const noexcept {
    auto it = m_entries.rbegin();
    while(it != m_entries.rend()) {
        auto [ptr, deleted] = *it;
        ++it;

        if(!deleted)
            co_yield ptr;
    }
}

qglk::object* qglk::object_list::next_impl(qglk::object* p_cur) const noexcept {
    assert(!p_cur || p_cur->m_index >= 0);
    assert(!p_cur || p_cur->get_type() == m_type);

    size_t idx;
    if(p_cur) {
        assert(static_cast<size_t>(p_cur->m_index) < m_entries.size() && m_entries[p_cur->m_index].ptr == p_cur);
        idx = p_cur->m_index + 1;
    } else if(get_entry_count() == 0) {
        return nullptr;
    } else {
        idx = 0;
    }

    while(idx < m_entries.size() && m_entries[idx].deleted)
        ++idx;

    if(idx < m_entries.size())
        return m_entries[idx].ptr;

    return nullptr;
}

void qglk::object_list::remove(qglk::object* p_t) {
    assert(p_t);
    assert(p_t->get_type() == m_type);
    assert(p_t->m_index >= 0);
    assert(!m_entries.empty());

    size_t idx = std::exchange(p_t->m_index, -1);
    if(idx == m_entries.size() - 1) {
        m_entries.pop_back();
    } else {
        m_entries[idx].deleted = true;
        m_free_list.push_back(idx);
    }
}

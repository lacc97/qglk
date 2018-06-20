#include "glk.hpp"

#include <QHash>

giblorb_map_t* s_BlorbMap = 0; /* NULL */

giblorb_err_t giblorb_set_resource_map(strid_t file) {
    giblorb_err_t err = giblorb_create_map(file, &s_BlorbMap);

    if(err) {
        s_BlorbMap = NULL;
        return err;
    }

    return giblorb_err_None;
}

giblorb_map_t* giblorb_get_resource_map() {
    return s_BlorbMap;
}

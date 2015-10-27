/* Copyright 2015 Realm Inc - All Rights Reserved
 * Proprietary and Confidential
 */

#ifndef REALM_LIST_HPP
#define REALM_LIST_HPP

#include "shared_realm.hpp"
#include <realm/link_view.hpp>

namespace realm {
    class Object;
    class List {
      public:
        List(SharedRealm &r, const ObjectSchema &s, LinkViewRef l) : m_realm(r), object_schema(s), m_link_view(l) {}

        const ObjectSchema &object_schema;
        SharedRealm realm() { return m_realm; }

        size_t size();

        Object get(std::size_t row_index);
        void add(std::size_t objct_index);
        void insert(std::size_t row_index, std::size_t objct_index);
        void set(std::size_t row_index, std::size_t objct_index);
        void remove(std::size_t row_index);

      private:
        SharedRealm m_realm;
        LinkViewRef m_link_view;

        void verify_attached();
        void verify_in_transaction();
        void verify_valid_row(std::size_t row_ndx, std::size_t max);
    };
}


#endif /* REALM_LIST_HPP */

/* Copyright 2015 Realm Inc - All Rights Reserved
 * Proprietary and Confidential
 */

#include "list.hpp"
#include "object_accessor.hpp"
#include <stdexcept>

using namespace realm;

size_t List::size()
{
    verify_attached();
    return m_link_view->size();
}

Object List::get(std::size_t row_ndx)
{
    verify_attached();
    verify_valid_row(row_ndx, m_link_view->size()-1);
    return Object(m_realm, object_schema, m_link_view->get(row_ndx));
}

void List::add(std::size_t object_index)
{
    insert(size(), object_index);
}

void List::insert(std::size_t row_index, std::size_t object_index)
{
    verify_in_transaction();
    verify_attached();
    verify_valid_row(row_index, m_link_view->size());
    m_link_view->insert(row_index, object_index);
}

void List::set(std::size_t row_index, std::size_t object_index)
{
    verify_in_transaction();
    verify_attached();
    verify_valid_row(row_index, m_link_view->size()-1);
    m_link_view->set(row_index, object_index);
}

void List::remove(std::size_t row_index)
{
    verify_in_transaction();
    verify_attached();
    m_link_view->remove(row_index);
}

void List::verify_valid_row(std::size_t row_ndx, std::size_t max)
{
    if (row_ndx > max) {
        throw std::out_of_range(std::string("Index ") + std::to_string(row_ndx) + " is outside of range 0..." + std::to_string(max) + ".");
    }
}

void List::verify_attached()
{
    if (!m_link_view->is_attached()) {
        throw std::runtime_error("Tableview is not attached");
    }
    m_link_view->sync_if_needed();
}

void List::verify_in_transaction()
{
    if (m_realm->is_in_transaction()) {
        throw MutationOutsideTransactionException("Attemping to change a List outside of a transacion.");
    }
}


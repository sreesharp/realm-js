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

Object List::get(std::size_t list_index)
{
    verify_attached();
    verify_valid_row(list_index, m_link_view->size()-1);
    return Object(m_realm, object_schema, m_link_view->get(list_index));
}

void List::add(std::size_t object_index)
{
    insert(size(), object_index);
}

void List::insert(std::size_t list_index, std::size_t row_index)
{
    verify_in_transaction();
    verify_attached();
    verify_valid_row(list_index, m_link_view->size());
    m_link_view->insert(list_index, row_index);
}

void List::set(std::size_t list_index, std::size_t row_index)
{
    verify_in_transaction();
    verify_attached();
    verify_valid_row(list_index, m_link_view->size()-1);
    m_link_view->set(list_index, row_index);
}

void List::remove(std::size_t list_index)
{
    verify_in_transaction();
    verify_attached();
    verify_valid_row(list_index, m_link_view->size()-1);
    m_link_view->remove(list_index);
}

void List::verify_valid_row(std::size_t list_index, std::size_t max)
{
    if (list_index > max) {
        throw std::out_of_range(std::string("Index ") + std::to_string(list_index) + " is outside of range 0..." + std::to_string(max) + ".");
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
    if (!m_realm->is_in_transaction()) {
        throw MutationOutsideTransactionException("Attemping to change a List outside of a transacion.");
    }
}


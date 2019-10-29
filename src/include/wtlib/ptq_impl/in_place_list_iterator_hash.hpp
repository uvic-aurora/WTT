#ifndef IN_PLACE_LIST_ITERATOR_HASH_HPP
#define IN_PLACE_LIST_ITERATOR_HASH_HPP

/**
 * @file     in_place_list_iterator_hash.hpp
 * @brief    Defines the hash function of in_place_list iterator (in case some dated
 *           CGAL does not have this).
 *
 * @copyright Copyright (c) 2019
 *
 */


#include <CGAL/In_place_list.h>
#include <functional>

#if defined (WTLIB_HASH_IN_PLACE_LIST_ITERATOR)
namespace std
{
  template < class T, class Alloc >
  struct hash<CGAL::internal::In_place_list_iterator<T, Alloc> >
    : public std::unary_function<CGAL::internal::In_place_list_iterator<T, Alloc>, std::size_t>  {

    std::size_t operator()(const CGAL::internal::In_place_list_iterator<T, Alloc>& i) const
    {
      const T* ptr = &*i;
      return reinterpret_cast<std::size_t>(ptr)/ sizeof(T);
    }
  };

  template < class T, class Alloc >
  struct hash<CGAL::internal::In_place_list_const_iterator<T, Alloc> >
    : public std::unary_function<CGAL::internal::In_place_list_const_iterator<T, Alloc>, std::size_t> {

    std::size_t operator()(const CGAL::internal::In_place_list_const_iterator<T, Alloc>& i) const
    {
      const T* ptr = &*i;
      return reinterpret_cast<std::size_t>(ptr)/ sizeof(T);
    }
  };
}
#endif  // if defined (WTLIB_HASH_IN_PLACE_LIST_ITERATOR)

#endif  // define IN_PLACE_LIST_ITERATOR_HASH_HPP
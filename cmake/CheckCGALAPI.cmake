message(STATUS "Check CGAL API compatability...")
set(CMAKE_REQUIRED_LIBRARIES ${CGAL_LIBRARY}
                             ${CMAKE_REQUIRED_LIBRARIES})
check_cxx_source_compiles("
  #include <CGAL/In_place_list.h>
  #include <unordered_map>
  using iterator = typename CGAL::internal::In_place_list_iterator<int, std::allocator<int>>;
  using const_iterator = typename CGAL::internal::In_place_list_const_iterator<int, std::allocator<int>>;
  int main()
  {
    std::unordered_map<iterator, int> map0;
    std::unordered_map<const_iterator, int> map1;
    return 0;
  }
" CGAL_HAS_IN_PLACE_LIST_ITERATOR_HASH)

if (NOT CGAL_HAS_IN_PLACE_LIST_ITERATOR_HASH)
  add_definitions(-DWTLIB_HASH_IN_PLACE_LIST_ITERATOR)
endif ()
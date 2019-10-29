#ifndef PTQ_IMPL_MESH_VERTEX_INFO_HPP
#define PTQ_IMPL_MESH_VERTEX_INFO_HPP

/**
 * @file     mesh_vertex_info.hpp
 * @brief    The implementation that will be used to instantiate Wavelet_mesh_ops.
 * 
 * @copyright Copyright (c) 2019
 * 
 */


#if defined (WTLIB_HASH_IN_PLACE_LIST_ITERATOR)
#include <wtlib/ptq_impl/in_place_list_iterator_hash.hpp>
#endif

#if defined (WTLIB_USE_UNORDERED_MAP)
#include <unordered_map>
#else
#include <map>
#endif

namespace wtlib::ptq_impl
{
#if !defined(WTLIB_USE_CUSTOM_MESH)
struct Vertex_info
{
  int id;
  int level;
  int type;
  bool border;
};
#endif
template <class Mesh>
class Mesh_info
{
public:
  using Vertex_const_handle = typename Mesh::Vertex_const_handle;
  using Vertex_handle = typename Mesh::Vertex_handle;

#if defined (WTLIB_USE_CUSTOM_MESH)
  int get_vertex_id(Vertex_const_handle v)
  {
    return v->id;
  }

  void set_vertex_id(Vertex_handle v, int id)
  {
    v->id = id;
  }

  int get_vertex_level(Vertex_const_handle v)
  {
    return v->level;
  }

  void set_vertex_level(Vertex_handle v, int level)
  {
    v->level = level;
  }

  int get_vertex_type(Vertex_const_handle v)
  {
    return v->type;
  }

  void set_vertex_type(Vertex_handle v, int type)
  {
    v->type = type;
  }

  bool get_vertex_border(Vertex_const_handle v)
  {
    return v->border;
  }

  void set_vertex_border(Vertex_handle v, bool border)
  {
    v->border = border;
  }
#else
  int get_vertex_id(Vertex_const_handle v)
  {
    return mesh_vertex_info_.at(v).id;
  }

  void set_vertex_id(Vertex_handle v, int id)
  {
    mesh_vertex_info_[v].id = id;
  }

  int get_vertex_level(Vertex_const_handle v)
  {
    return mesh_vertex_info_.at(v).level;
  }

  void set_vertex_level(Vertex_handle v, int level)
  {
    mesh_vertex_info_[v].level = level;
  }

  int get_vertex_type(Vertex_const_handle v)
  {
    return mesh_vertex_info_.at(v).type;
  }

  void set_vertex_type(Vertex_handle v, int type)
  {
    mesh_vertex_info_[v].type = type;
  }

  bool get_vertex_border(Vertex_const_handle v)
  {
    return mesh_vertex_info_.at(v).border;
  }

  void set_vertex_border(Vertex_handle v, bool border)
  {
    mesh_vertex_info_[v].border = border;
  }
#endif

private:
#if !defined(WTLIB_USE_CUSTOM_MESH) 
#if defined (WTLIB_USE_UNORDERED_MAP)
  std::unordered_map<Vertex_const_handle, Vertex_info> mesh_vertex_info_;
#else
  std::map<Vertex_const_handle, Vertex_info> mesh_vertex_info_;
#endif
#endif
};  // define class Mesh_info

}  // define wtlib::ptq_impl
#endif   // define PTQ_IMPL_MESH_VERTEX_INFO_HPP
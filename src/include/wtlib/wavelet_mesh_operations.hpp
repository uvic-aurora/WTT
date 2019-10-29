#ifndef WAVELET_MESH_OPERATIONS_HPP
#define WAVELET_MESH_OPERATIONS_HPP

namespace wtlib
{
template<class M,
         class F1,
         class F2,
         class F3,
         class F4,
         class F5,
         class F6,
         class F7,
         class F8>
class Wavelet_mesh_operations
{
public:
  using Mesh = M;
  using Get_vertex_id = F1;
  using Set_vertex_id = F2;
  using Get_vertex_level = F3;
  using Set_vertex_level = F4;
  using Get_vertex_type = F5;
  using Set_vertex_type = F6;
  using Get_vertex_border = F7;
  using Set_vertex_border = F8;

  Wavelet_mesh_operations(
    Get_vertex_id get_vertex_id,
    Set_vertex_id set_vertex_id,
    Get_vertex_level get_vertex_level,
    Set_vertex_level set_vertex_level,
    Get_vertex_type get_vertex_type,
    Set_vertex_type set_vertex_type,
    Get_vertex_border get_vertex_border,
    Set_vertex_border set_vertex_border
  ) :
    get_vertex_id_(get_vertex_id),
    set_vertex_id_(set_vertex_id),
    get_vertex_level_(get_vertex_level),
    set_vertex_level_(set_vertex_level),
    get_vertex_type_(get_vertex_type),
    set_vertex_type_(set_vertex_type),
    get_vertex_border_(get_vertex_border),
    set_vertex_border_(set_vertex_border)
  {}

  int get_vertex_id(typename Mesh::Vertex_const_handle v) const {
    return get_vertex_id_(v);
  }
  void set_vertex_id(typename Mesh::Vertex_handle v, int id) const {
    set_vertex_id_(v, id);
  }
  int get_vertex_level(typename Mesh::Vertex_const_handle v) const {
    return get_vertex_level_(v);
  }
  void set_vertex_level(typename Mesh::Vertex_handle v, int level) const {
    set_vertex_level_(v, level);
  }
  int get_vertex_type(typename Mesh::Vertex_const_handle v) const {
    return get_vertex_type_(v);
  }
  void set_vertex_type(typename Mesh::Vertex_handle v, int type) const {
    set_vertex_type_(v, type);
  }
  bool get_vertex_border(typename Mesh::Vertex_const_handle v) const {
    return get_vertex_border_(v);
  }
  void set_vertex_border(typename Mesh::Vertex_handle v, bool border) const {
    set_vertex_border_(v, border);
  }
private:
  Get_vertex_id get_vertex_id_;
  Set_vertex_id set_vertex_id_;
  Get_vertex_level get_vertex_level_;
  Set_vertex_level set_vertex_level_;
  Get_vertex_type get_vertex_type_;
  Set_vertex_type set_vertex_type_;
  Get_vertex_border get_vertex_border_;
  Set_vertex_border set_vertex_border_;
};  // class Wavelet_mesh_operations


}  // namespace wtlib

#endif  // define WAVELET_MESH_OPERATIONS_HPP
#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP
#include <wtlib/mesh_types.hpp>
#include <wtlib/ptq_impl/mesh_vertex_info.hpp>
#include <wtlib/ptq_impl/subdivision_modifier.hpp>


#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>
#include <iostream>
#include <fstream>
#include <regex>

namespace fs = boost::filesystem;

template<class Mesh, class Mesh_ops>
class Wtlib_test_helper
{
public:
  using Mesh_info = wtlib::ptq_impl::Mesh_info<Mesh>;
  using Modifier = wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  using Halfedge_pair = typename Modifier::Halfedge_pair;
  using Point = typename Mesh::Traits::Point_3;

  static Mesh_ops initMeshOps()
  {
    using std::placeholders::_1;
    using std::placeholders::_2;
    std::shared_ptr<Mesh_info> m_info_ptr {std::make_shared<Mesh_info>()};
    return Mesh_ops {
      std::bind(&Mesh_info::get_vertex_id, m_info_ptr, _1),
      std::bind(&Mesh_info::set_vertex_id, m_info_ptr, _1, _2),
      std::bind(&Mesh_info::get_vertex_level, m_info_ptr, _1),
      std::bind(&Mesh_info::set_vertex_level, m_info_ptr, _1, _2),
      std::bind(&Mesh_info::get_vertex_type, m_info_ptr, _1),
      std::bind(&Mesh_info::set_vertex_type, m_info_ptr, _1, _2),
      std::bind(&Mesh_info::get_vertex_border, m_info_ptr, _1),
      std::bind(&Mesh_info::set_vertex_border, m_info_ptr, _1, _2),
    };
  }

  static void initMeshInfo(Mesh& m, const Mesh_ops& m_ops)
  {
    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      m_ops.set_vertex_id(v, id);
      m_ops.set_vertex_border(v, false);
      m_ops.set_vertex_level(v, 0);
      m_ops.set_vertex_type(v, 0);
    }

    if (m.normalized_border_is_valid())
    {
      for (auto h = m.border_halfedges_begin(); h != m.halfedges_end(); ++h)
      {
        m_ops.set_vertex_border(h->vertex(), true);
        m_ops.set_vertex_border(h->opposite()->vertex(), true);
      }
    }
    else
    {
      for (auto h = m.halfedges_begin(); h != m.halfedges_end(); ++h)
      {
        if (h->is_border())
        {
          m_ops.set_vertex_border(h->vertex(), true);
          m_ops.set_vertex_border(h->opposite()->vertex(), true);          
        }
      }
    }
  }

  static std::vector<int> getSubdivisionLevels(const std::string& file)
  {
    std::vector<int> v_size_in_levels;

    std::smatch match_res;
  
    std::string target = file;
    std::regex regex(R"(_\d+)");

    while (std::regex_search(target, match_res, regex))
    {
      int size = std::stoi(match_res.str().erase(0, 1));
      assert(size > 0);
      v_size_in_levels.push_back(size);
      // This line will invalidate match_res's iterator.
      target = match_res.suffix();
    }
    return v_size_in_levels;
  }

  static void loadFiles(std::vector<std::string>& files,
                 const std::string& dir)
  {
    fs::path data_dir(dir);
    assert(fs::exists(data_dir));
    assert(fs::is_directory(data_dir));

    fs::directory_iterator iter(data_dir);
    fs::directory_iterator end_iter;

    while (iter != end_iter)
    {
      if (   fs::is_regular_file(*iter)
          && iter->path().extension() == std::string(".off"))
      {
        files.push_back(iter->path().string());
      }
      ++iter;
    }
  }

  static Mesh loadMesh(const std::string& file)
  {
    std::ifstream mf;
    mf.open(file);
    if (!mf.is_open())
    {
      std::cerr << file << " not found\n";
      assert(false);
      return Mesh {};
    }
    Mesh m;
    mf >> m;
    mf.close();
    return m;
  }


  static void loop_subdivision_step(Mesh& mesh, const Mesh_ops& m_ops, int current_level)
  {
    using Point = typename Mesh::Traits::Point_3;
    using Edge_iterator = typename Mesh::Edge_iterator;

    int edge_size = mesh.size_of_halfedges() / 2;
    int vertices_size = mesh.size_of_vertices();

    std::vector<Point> points_buffer;
    points_buffer.reserve(edge_size + vertices_size);

    for (Vertex_handle v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v)
    {
      points_buffer.emplace_back(v->point());
    }
    assert(points_buffer.size() == mesh.size_of_vertices());

    for (int i = 0; i < edge_size; ++i)
    {
      points_buffer.emplace_back(CGAL::ORIGIN);
    }

    auto edge_compair = [&m_ops](const Edge_iterator& lhs,
                               const Edge_iterator& rhs) -> bool
                      {
                        int s_lhs = m_ops.get_vertex_id(lhs->vertex());
                        int l_lhs = m_ops.get_vertex_id(lhs->opposite()->vertex());
                        if (s_lhs > l_lhs)
                        {
                          std::swap(s_lhs, l_lhs);
                        }

                        int s_rhs = m_ops.get_vertex_id(rhs->vertex());
                        int l_rhs = m_ops.get_vertex_id(rhs->opposite()->vertex());
                        if (s_rhs > l_rhs)
                        {
                          std::swap(s_rhs, l_rhs);
                        }

                        return s_lhs == s_rhs ? l_lhs < l_rhs : s_lhs < s_rhs;
                      };

    // Store mesh edges in a sorted order.
    std::set<Edge_iterator, decltype(edge_compair)> edges_set(edge_compair);

    for (Edge_iterator e = mesh.edges_begin(); e != mesh.edges_end(); ++e)
    {
      edges_set.insert(e);
    }

    // Edge vertex
    for (auto [eitr, idx] = std::make_pair(edges_set.begin(), 0);
         eitr != edges_set.end(); ++eitr, ++idx)
    {
      Halfedge_handle h = *eitr;
      Point& e = points_buffer[idx + vertices_size];
      loop_edge_point(h, e);
    }

    // Old vertex
    for (auto [v, idx] = std::make_pair(mesh.vertices_begin(), 0);
         v != mesh.vertices_end(); ++v, ++idx)
    {
      Point& pt = points_buffer[idx];
      loop_vertex_point(v, pt, m_ops);
    }

    // Topology refine
    std::vector<Vertex_handle> vertices;
    vertices.reserve(vertices_size + edge_size);
    std::vector<Vertex_handle*> bands;
    bands.resize(3);
    for (Vertex_handle v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v)
    {
      vertices.push_back(v);
    }
    bands.push_back(&vertices.front());
    bands.push_back(&vertices.back() + 1);
    Modifier::refine(mesh, m_ops, current_level, vertices, bands);

    // Assign points from buffer to vertices
    for (int i = 0; i < vertices.size(); ++i)
    {
      Vertex_handle v = vertices[i];
      v->point() = points_buffer[i];
    }
  }


  static void butterfly_subdivision_step(Mesh& mesh, const Mesh_ops& m_ops, int current_level)
  {
    using Point = typename Mesh::Traits::Point_3;
    using Edge_iterator = typename Mesh::Edge_iterator;

    int edge_size = mesh.size_of_halfedges() / 2;
    int vertices_size = mesh.size_of_vertices();

    std::vector<Point> points_buffer;
    points_buffer.reserve(edge_size + vertices_size);

    for (Vertex_handle v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v)
    {
      points_buffer.emplace_back(v->point());
    }

    for (int i = 0; i < edge_size; ++i)
    {
      points_buffer.emplace_back(CGAL::ORIGIN);
    }

    auto edge_compair = [&m_ops](const Edge_iterator& lhs,
                               const Edge_iterator& rhs) -> bool
                      {
                        int s_lhs = m_ops.get_vertex_id(lhs->vertex());
                        int l_lhs = m_ops.get_vertex_id(lhs->opposite()->vertex());
                        if (s_lhs > l_lhs)
                        {
                          std::swap(s_lhs, l_lhs);
                        }

                        int s_rhs = m_ops.get_vertex_id(rhs->vertex());
                        int l_rhs = m_ops.get_vertex_id(rhs->opposite()->vertex());
                        if (s_rhs > l_rhs)
                        {
                          std::swap(s_rhs, l_rhs);
                        }

                        return s_lhs == s_rhs ? l_lhs < l_rhs : s_lhs < s_rhs;
                      };

    // Store mesh edges in a sorted order.
    std::set<Edge_iterator, decltype(edge_compair)> edges_set(edge_compair);

    for (Edge_iterator e = mesh.edges_begin(); e != mesh.edges_end(); ++e)
    {
      edges_set.insert(e);
    }
    // Edge vertex
    for (auto [eitr, idx] = std::make_pair(edges_set.begin(), 0);
         eitr != edges_set.end(); ++eitr, ++idx)
    {
      Halfedge_handle h = *eitr;
      Point& e = points_buffer[idx + vertices_size];
      butterfly_edge_point(h, e);
    }

    // Topology refine
    std::vector<Vertex_handle> vertices;
    vertices.reserve(vertices_size + edge_size);
    std::vector<Vertex_handle*> bands;
    bands.reserve(3);
    for (Vertex_handle v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v)
    {
      vertices.push_back(v);
    }
    bands.push_back(&vertices.front());
    bands.push_back(&vertices.back() + 1);
    Modifier::refine(mesh, m_ops, current_level, vertices, bands);
    assert(vertices.size() == mesh.size_of_vertices());

    // Assign points from buffer to vertices
    for (int i = 0; i < vertices.size(); ++i)
    {
      Vertex_handle v = vertices[i];
      v->point() = points_buffer[i];
    }
  }


  static void loop_edge_point(Halfedge_handle h, Point& p)
  {
    if (!h->is_border_edge())
    {
      Point& p1 = h->vertex()->point();
      Point& p2 = h->opposite()->vertex()->point();
      Point& f1 = h->next()->vertex()->point();
      Point& f2 = h->opposite()->next()->vertex()->point();

      p = Point((3*(p1[0]+p2[0])+f1[0]+f2[0])/8,
         (3*(p1[1]+p2[1])+f1[1]+f2[1])/8,
         (3*(p1[2]+p2[2])+f1[2]+f2[2])/8 );
    }
    else
    {
      Point& p1 = h->vertex()->point();
      Point& p2 = h->opposite()->vertex()->point();
      p = Point((p1[0]+p2[0])/2, (p1[1]+p2[1])/2, (p1[2]+p2[2])/2);
    }
  }


  static void loop_vertex_point(Vertex_handle v, Point& pt, const Mesh_ops& m_ops)
  {
    using FT = typename Mesh::Traits::Kernel::FT;

    Point S = v->point();
    if (m_ops.get_vertex_border(v))
    {
      typename Modifier::Halfedge_pair hp = Modifier::get_halfedges_to_borders(v);
      Point p0 = hp.first->vertex()->point();
      Point p1 = hp.second->vertex()->point();
      pt = Point((p0[0] + 6.0 * S[0] + p1[0]) / 8.0, 
                 (p0[1] + 6.0 * S[1] + p1[1]) / 8.0,
                 (p0[2] + 6.0 * S[2] + p1[2]) / 8.0);
    }
    else
    {
      typename Mesh::Halfedge_around_vertex_circulator vcir = v->vertex_begin();
      std::size_t n = v->degree();

      FT R[] = {0.0, 0.0, 0.0};
      
      for (size_t i = 0; i < n; i++, ++vcir)
      {
        Point& p = vcir->opposite()->vertex()->point();
        R[0] += p[0];   R[1] += p[1];   R[2] += p[2];
      }

      if (n == 6) 
      {
        pt = Point((10*S[0]+R[0])/16, (10*S[1]+R[1])/16, (10*S[2]+R[2])/16);
      }
      else 
      {
        FT Cn = (FT) (5.0/8.0 - CGAL::square(3+2*std::cos(2 * CGAL_PI/(double) n))/64.0);
        FT Sw = (double)n*(1-Cn)/Cn;
        FT W = (double)n/Cn;
        pt = Point((Sw*S[0]+R[0])/W, (Sw*S[1]+R[1])/W, (Sw*S[2]+R[2])/W);
      }
    }
  }


  static void butterfly_edge_point(Halfedge_handle edge, Point& p)
  {
    Point& a0 = edge->vertex()->point();
    Point& a1 = edge->opposite()->vertex()->point();

    Point& b0 = edge->next()->vertex()->point();
    Point& b1 = edge->opposite()->next()->vertex()->point();

    Point& c0 = edge->next()->opposite()->next()->vertex()->point();
    Point& c1 = edge->opposite()->prev()->opposite()->next()->vertex()->point();

    Point& c2 = edge->opposite()->next()->opposite()->next()->vertex()->point();
    Point& c3 = edge->prev()->opposite()->next()->vertex()->point();

    p  = Point(0.5 * (a0[0] + a1[0]) + 0.125 * (b0[0] + b1[0]) - 0.0625 * (c0[0] + c1[0] + c2[0] + c3[0]),
               0.5 * (a0[1] + a1[1]) + 0.125 * (b0[1] + b1[1]) - 0.0625 * (c0[1] + c1[1] + c2[1] + c3[1]),
               0.5 * (a0[2] + a1[2]) + 0.125 * (b0[2] + b1[2]) - 0.0625 * (c0[2] + c1[2] + c2[2] + c3[2]));
  }
};
#endif
#include <wtlib/ptq_impl/mesh_vertex_info.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>
#include <wtlib/mesh_types.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <functional>
#include <memory>

namespace po = boost::program_options;

using Vertex_handle = typename Mesh::Vertex_handle;
using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Halfedge_handle = typename Mesh::Halfedge_handle;
using Halfedge_const_handle = typename Mesh::Halfedge_const_handle;
using Get_vertex_id = std::function<int(Vertex_const_handle)>;
using Set_vertex_id = std::function<void(Vertex_handle, int)>;
using Get_vertex_level = std::function<int(Vertex_const_handle)>;
using Set_vertex_level = std::function<void(Vertex_handle, int)>;
using Get_vertex_type = std::function<int(Vertex_const_handle)>;
using Set_vertex_type = std::function<void(Vertex_handle, int)>;
using Get_vertex_border = std::function<bool(Vertex_const_handle)>;
using Set_vertex_border = std::function<void(Vertex_handle, bool)>;

using Mesh_info = wtlib::ptq_impl::Mesh_info<Mesh>;

using Mesh_ops = wtlib::Wavelet_mesh_operations<
                                Mesh,
                                Get_vertex_id,
                                Set_vertex_id,
                                Get_vertex_level,
                                Set_vertex_level,
                                Get_vertex_type,
                                Set_vertex_type,
                                Get_vertex_border,
                                Set_vertex_border>;

Mesh_ops create_mesh_ops()
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

void dump_mesh(const Mesh& mesh, const Mesh_ops& mesh_ops, const std::vector<Vertex_handle>& sorted_vertices, std::ostream& out)
{
  out << "OFF\n";
  out << mesh.size_of_vertices() << " " << mesh.size_of_facets() << " " << "0\n";
  for (auto v : sorted_vertices)
  {
    out << v->point().x() << " " << v->point().y() << " " << v->point().z() << "\n";
  }

  for (auto f = mesh.facets_begin(); f != mesh.facets_end(); ++f)
  {
    Halfedge_const_handle h = f->facet_begin();
    out << "3 ";
    do
    {
      out << mesh_ops.get_vertex_id(h->vertex());
      if (h->next() != f->facet_begin())
      {
        out << " ";
      }
      else
      {
        out << "\n";
      }
      h = h->next();
    }
    while (h != f->facet_begin());
  }
}

int main(int argc, char** argv)
{
  po::options_description desc("Sort mesh vertices based on subdivision connectivity");

  desc.add_options()
    ("help,h", "Usage. \n")
    ("level,l", po::value<int>(), "Set subdivision levels.\n\n")
    ("input-mesh,i", po::value<std::string>(), "Set file path of input mesh.\n\n"
                                               "Without this option, program will read input mesh from standard input.\n\n")
    ("output-mesh,o", po::value<std::string>(), "Set file path of output sorted mesh.\n\n"
                                                "Without this option, program will print output mesh to standard output.\n\n");

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);

  std::vector<std::string> unknown_opts =
      po::collect_unrecognized(parsed.options, po::include_positional);
  
  if (!unknown_opts.empty())
  {
    std::cerr << "Unknown option: " << unknown_opts[0] << '\n';
    return 1;
  }

  int num_levels = 0;
  std::string mesh_in;
  std::string mesh_out;

  // Parse command line options
  if (vm.count("help"))
  {
    std::cout << desc;
    return 0;
  }

  if (vm.count("level"))
  {
    num_levels = vm["level"].as<int>();
  }
  else
  {
    std::cerr << "Please set wavelet transform level.\n";
    return 1;
  }

  if (vm.count("input-mesh"))
  {
    mesh_in = vm["input-mesh"].as<std::string>();
  }

  if (vm.count("output-mesh"))
  {
    mesh_out = vm["output-mesh"].as<std::string>();
  }


  // Validate mesh.
  Mesh mesh;

  if (mesh_in.empty()) {
    if (!(std::cin >> mesh)) {
      std::cerr << "Fail to read mesh\n";
      return 1;
    }
  } else {
    std::ifstream mesh_in_file(mesh_in);
    if (!(mesh_in_file) || !(mesh_in_file >> mesh)) {
      std::cerr << "Fail to read mesh from " << mesh_in << '\n';
      return 1;
    }
  }

  // Initialize mesh border info
  Mesh_ops mesh_ops {create_mesh_ops()};
  for (auto v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v) 
  {
    mesh_ops.set_vertex_border(v, false);
  }
    // Then, set the border flag to true for each border vertex.
  if (mesh.normalized_border_is_valid())
  {
    // The border is normalized so we can handle this more efficiently.
    for (auto h = mesh.border_halfedges_begin(); h != mesh.halfedges_end(); ++h)
    {
      mesh_ops.set_vertex_border(h->vertex(), true);
      mesh_ops.set_vertex_border(h->opposite()->vertex(), true);
    }
  } 
  else 
  {
    // The border is not normalized so we must use a less efficient algorithm.
    for (auto h = mesh.halfedges_begin(); h != mesh.halfedges_end(); ++h)
    {
      if (h->is_border_edge())
      {
        mesh_ops.set_vertex_border(h->vertex(), true);
        mesh_ops.set_vertex_border(h->opposite()->vertex(), true);
      }
    }
  }

  std::vector<typename Mesh::Vertex_handle> vertices;
  vertices.reserve(mesh.size_of_vertices());
  std::vector<typename Mesh::Vertex_handle*> bands;
  bands.reserve(num_levels + 2);
  
  bool res = wtlib::ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>::classify(mesh, mesh_ops, num_levels, vertices, bands, true);

  if (!res)
  {
    std::cerr << "Mesh does not have " << num_levels << " levels subdivision connectivity\n";
    return 1;
  }

  if (mesh_out.empty())
  {
    dump_mesh(mesh, mesh_ops, vertices, std::cout);
  }
  else
  {
    std::ofstream mesh_out_file(mesh_out);
    if (!mesh_out_file.is_open())
    {
      std::cerr << "Fail to open file " << mesh_out << " to write\n";
      return 1;
    }
    dump_mesh(mesh, mesh_ops, vertices, mesh_out_file);
    mesh_out_file.close();
  }

  return 0;
}
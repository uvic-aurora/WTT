#include <wtlib/wavelet_mesh_operations.hpp>
#include <wtlib/mesh_types.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <random>

namespace po = boost::program_options;
using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Halfedge_handle = typename Mesh::Halfedge_handle;
using Vertex_handle = typename Mesh::Vertex_handle;
using Halfedge_around_vertex_circulator = Mesh::Halfedge_around_vertex_circulator;
using Point = typename Mesh::Traits::Point_3;
using Vector = typename Mesh::Traits::Vector_3;


void add_noise_to_normal(Mesh& mesh, double mean, double deviation)
{
  std::random_device r_rd {};
  std::mt19937_64 r_gen {r_rd()};
  std::normal_distribution<> r_dis {mean, deviation};
  std::uniform_real_distribution<> theta_dis {-M_PI, M_PI};
  std::uniform_real_distribution<> phi_dis {-M_PI, M_PI};

  for (Vertex_handle v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v)
  {
    double r = r_dis(r_gen);
    double theta = theta_dis(r_gen);
    double phi = phi_dis(r_gen);
    
    Point p0 = v->point();
    v->point() = Point {p0.x() + r * std::sin(theta) * std::cos(phi),
                        p0.y() + r * std::sin(theta) * std::sin(phi),
                        p0.z() + r * std::cos(theta)};
  }
}


int main(int argc, char** argv)
{
  po::options_description descriptions("Add uniform distribution noise to mesh");
  descriptions.add_options()
    ("help,h", "Display usage.\n\n")
    ("input-mesh,i", po::value<std::string>(), "Set file path of input mesh.\n\n"
                                               "Without this option, program will read input mesh from standard input.\n\n")
    ("output-mesh,o", po::value<std::string>(), "Set file path of output noisy mesh.\n\n"
                                                "Without this option, program will print output mesh to standard output.\n\n")
    ("deviation,d", po::value<double>(), "Set noise standard deviation\n\n"
                                           "Default: 1.0\n\n")
    ("mean,m", po::value<double>(), "Set noise mean value\n\n"
                                           "Default:  0.0\n\n");

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv)
                                  .options(descriptions)
                                  .allow_unregistered()
                                  .run();
  try
  {
    po::store(parsed, vm);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
  po::notify(vm);

  std::vector<std::string> unknown_opts =
      po::collect_unrecognized(parsed.options, po::include_positional);

  if (!unknown_opts.empty()) {
    std::cerr << "Unknown option: " << unknown_opts[0] << '\n';
    return 1;
  }

  std::string mesh_in;
  std::string mesh_out;
  int level = 0;
  int max_level = 0;
  double mean =  0.0;
  double deviation = 1.0;

  // Parse command line options
  if (vm.count("help"))
  {
    std::cout << descriptions;
    return 0;
  }

  if (vm.count("input-mesh"))
  {
    mesh_in = vm["input-mesh"].as<std::string>();
  }

  if (vm.count("output-mesh"))
  {
    mesh_out = vm["output-mesh"].as<std::string>();
  }

  if (vm.count("mean"))
  {
    mean = vm["mean"].as<double>();
  }

  if (vm.count("deviation"))
  {
    deviation = vm["deviation"].as<double>();
  }


  // Load mesh
  Mesh mesh;

  if (mesh_in.empty())
  {
    if (!(std::cin >> mesh))
    {
      std::cerr << "Fail to read mesh from stdin\n";
      return 1;
    }
  }
  else
  {
    std::ifstream mesh_in_file(mesh_in);
    if (!(mesh_in_file) || !(mesh_in_file >> mesh))
    {
      std::cerr << "Fail to read mesh from " << mesh_in << '\n';
      return 1;
    }
    mesh_in_file.close();
  }

  add_noise_to_normal(mesh, mean, deviation);

  // Output noisy mesh
  if (mesh_out.empty())
  {
    std::cout << mesh;
    std::cout << '\n';
  }
  else
  {
    std::ofstream mesh_out_file(mesh_out);
    if (!mesh_out_file.is_open())
    {
      std::cerr << "Fail to open file " << mesh_out << " to write\n";
      return 1;
    }
    mesh_out_file << mesh;
    mesh_out_file.close();
  }
}
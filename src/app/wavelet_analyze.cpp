#include <wtlib/butterfly_wavelet_transform.hpp>
#include <wtlib/loop_wavelet_transform.hpp>
#include <wtlib/mesh_types.hpp>

#include <boost/program_options.hpp>

#include <chrono>
#include <fstream>

namespace po = boost::program_options;

using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Vertex_handle = typename Mesh::Vertex_handle;
using Vector3 = typename Mesh::Traits::Vector_3;

void dump_coefs(const std::vector<std::vector<Vector3>>& coefs,
                std::ostream& out)
{
  for (int i = 0; i < coefs.size(); ++i)
  {
    const std::vector<Vector3>& band_coefs = coefs[i];
    for (int j = 0; j < band_coefs.size(); ++j)
    {
      const Vector3& c = band_coefs[j];
      out << c.x() << " " << c.y() << " " << c.z() <<'\n';
    }
    out << "\n";
  }
}

int main(int argc, char** argv)
{
  po::options_description descriptions(R"(A program computes the Loop or Butterfly forward wavelet transform on a triangle mesh.

Usage:
    wtl_wavelet_analyze -m <scheme> -l <level> 
                        [--input-mesh <args>] [--output-mesh <args>]
                        [--output-coefs <args>]

These are accepted options)");
  descriptions.add_options()
    ("help,h", "Display the usage.\n")
    ("method,m", po::value<std::string>(), "Select a wavelet transform scheme:\n"
                                           "\t - Butterfly\n"
                                           "\t - Loop")
    ("level,l", po::value<int>(), "Set the number of wavelet transform levels.")
    ("input-mesh,i", po::value<std::string>(), "Set the file path for the input mesh. "
                                               "Without this option, the program will read input mesh from standard input.")
    ("output-mesh,o", po::value<std::string>(), "Set the file path for the coarse output mesh. "
                                                "Without this option, program will output mesh to standard output.")
    ("output-coefs,c", po::value<std::string>(), "Set the file path for the output wavelet coefficients. "
                                                 "Without this option, program will output wavelet coefficients to standard output.");


  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(descriptions).allow_unregistered().run();
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
  
  if (!unknown_opts.empty())
  {
    std::cerr << "Unknown option: " << unknown_opts[0] << '\n';
    return 1;
  }

  int num_levels = 0;
  std::string mesh_out;
  std::string coefs_out;
  std::string mesh_in;
  std::string method;

  // Parse command line options
  if (vm.count("help"))
  {
    std::cout << descriptions;
    return 0;
  }

  if (vm.count("method"))
  {
    method = vm["method"].as<std::string>();
    if (method != "Loop" && method != "Butterfly")
    {
      std::cerr << method << " is not supported, please select one from below:\n"
                             << "\t - Butterfly\n"
                             << "\t - Loop\n";
      return 1;
    }
  }
  else
  {
    std::cerr << "Please select a wavelet transform scheme from below: \n"
                 "\t - Butterfly\n"
                 "\t - Loop.\n";
    return 1;
  }

  if (vm.count("level"))
  {
    num_levels = vm["level"].as<int>();
    if (num_levels < 0)
    {
      std::cerr << "The set number of levels (" << num_levels << ") should be a non-negative integer.\n";
      return 1;
    }
  }
  else
  {
    std::cerr << "Please set the number of wavelet transform levels.\n";
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

  if (vm.count("output-coefs"))
  {
    coefs_out = vm["output-coefs"].as<std::string>();
  }

  // Load mesh
  Mesh mesh;

  if (mesh_in.empty())
  {
    if (!(std::cin >> mesh))
    {
      std::cerr << "[ERROR] Fail to read mesh from stdin.\n";
      return 1;
    }
  }
  else
  {
    std::ifstream mesh_in_file(mesh_in);
    if (!(mesh_in_file) || !(mesh_in_file >> mesh))
    {
      std::cerr << "[ERROR] Fail to read mesh from " << mesh_in << ".\n";
      return 1;
    }
    mesh_in_file.close();
  }

  if (method == "Butterfly")
  {
    if (!mesh.is_closed())
    {
      std::cerr << "[ERROR] A mesh with boundaries is not supported by the Butterfly wavelet transform!\n";
      return 1;
    }
  }

  std::vector<std::vector<Vector3>> coefs;

  if (method == "Butterfly")
  {
    if (!wtlib::butterfly_analyze(mesh, coefs, num_levels))
    {
      std::cerr << "[ERROR] The input mesh does not have " << num_levels 
                << " levels of subdivision connectivity.\n";
      return 1;
    }
  }
  else
  {
    if (!wtlib::loop_analyze(mesh, coefs, num_levels))
    {
      std::cerr << "[ERROR] The input mesh does not have " << num_levels 
                << " levels of subdivision connectivity.\n";
      return 1;
    }
  }

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
      std::cerr << "[ERROR] Fail to open file " << mesh_out << " to write mesh.\n";
      return 1;
    }
    mesh_out_file << mesh;
    mesh_out_file.close();
  }

  if (coefs_out.empty())
  {
    dump_coefs(coefs, std::cout);
  }
  else
  {
    std::ofstream coefs_out_file(coefs_out);
    if (!coefs_out_file.is_open())
    {
      std::cerr << "[ERROR] Fail to open file " << mesh_out << " to write coefficients.\n";
      return 1;
    }
    dump_coefs(coefs, coefs_out_file);
    coefs_out_file.close();
  }

  return 0;
}
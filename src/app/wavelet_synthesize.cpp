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
using Get_mesh_size = std::function<int(Mesh&, int)>;

void load_coefs(std::vector<std::vector<Vector3>>& coefs,
                std::istream& in)
{
  std::istringstream coef_scanner;
  std::string coef_buffer;
  int band_idx = 0;
  coefs.emplace_back(std::vector<Vector3>{});

  double x;
  double y;
  double z;

  while (std::getline(in, coef_buffer))
  {
    if (coef_buffer.empty())
    {
      ++band_idx;
      coefs.push_back(std::vector<Vector3>{});
    }
    else
    {
      coef_scanner.str(coef_buffer);
      if (!(coef_scanner >> x >> y >> z))
      {
        return;
      }

      coef_scanner.clear();

      Vector3 c {x, y, z};
      coefs[band_idx].emplace_back(c);
    }
  }

  while (coefs.back().empty())
  {
    coefs.pop_back();
  }
}

void coefficients_padding(std::vector<std::vector<Vector3>>& coefs,
                          Mesh& mesh,
                          int num_levels)
{
  if (coefs.size() != num_levels)
  {
    coefs.resize(num_levels);
  }

  // int act as a placeholder for Mesh_ops since it is not used
  Get_mesh_size get_mesh_size = wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, int>::ptq_mesh_size;

  for (int i = 0; i < coefs.size(); ++i)
  {
    int expect_size = get_mesh_size(mesh, i + 1) - get_mesh_size(mesh, i);
    if (coefs[i].size() != expect_size)
    {
      // Feeding zero coefficients
      coefs[i].resize(expect_size, Vector3 {0.0, 0.0, 0.0});
    }
  }
}

bool coefs_precheck(const std::vector<std::vector<Vector3>>& coefs,
                    Mesh& mesh,
                    int num_levels)
{
  if (coefs.size() != num_levels)
  {
    return false;
  }

  // int act as a placeholder for Mesh_ops since it is not used
  Get_mesh_size get_mesh_size = wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, int>::ptq_mesh_size;

  for (int i = 0; i < coefs.size(); ++i)
  {
    int expect_size = get_mesh_size(mesh, i + 1) - get_mesh_size(mesh, i);
    if (coefs[i].size() != expect_size)
    {
      std::cerr << "Invalid coefficients size in level " << i + 1 << '\n';
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv)
{
  po::options_description descriptions(R"(A program computes the Loop or Butterfly inverse wavelet transform on a triangle mesh.

Usage:
    wtl_wavelet_synthesize -m <scheme> -l <level> [-A]
                        [--input-mesh <args>] [--output-mesh <args>]
                        [--output-coefs <args>]

These are accepted options)");
  descriptions.add_options()
    ("help,h", "Display usage.\n")
    ("method,m", po::value<std::string>(), "Select a wavelet transform scheme:\n"
                                           "\t - Butterfly\n"
                                           "\t - Loop.")
    ("level,l", po::value<int>(), "Set the number of wavelet transform levels.")
    ("input-mesh,i", po::value<std::string>(), "Set the file path for the input mesh. "
                                               "Without this option, program will read input mesh from standard input.")
    ("output-mesh,o", po::value<std::string>(), "Set the file path for the refined output mesh. "
                                                "Without this option, program will output the computed mesh to standard output.")
    ("input-coefs,c", po::value<std::string>(), "Set the file path for the input wavelet coefficients. "
                                                "Without this option, program will read wavelet coefficients from standard input.")
    (",A", "Enable wavelet coefficient auto-padding. "
          "Enabling this option automatically adds zeros on insufficient wavelet coefficients or truncate redundant wavelet coefficients.");

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

  int num_levels = 0;
  std::string mesh_out;
  std::string coefs_in;
  std::string mesh_in;
  std::string method;
  bool auto_padding = false;

  // Parse command line options
  if (vm.count("help"))
  {
    std::cout << descriptions;
    return 0;
  }

  if (vm.count("A"))
  {
    auto_padding = true;
  }

  if (vm.count("method"))
  {
    method = vm["method"].as<std::string>();
    if (method != "Loop" && method != "Butterfly")
    {
      std::cerr << method << " is not supported, please select one from below: \n"
                             << "\t - Butterfly\n"
                             << "\t - Loop\n";
      return 1;
    }
  }
  else
  {
    std::cerr << "Please select a wavelet transform scheme from below: \n"
                 "\t - Butterfly\n"
                 "\t - Loop\n";
    return 1;
  }

  if (vm.count("level"))
  {
    num_levels = vm["level"].as<int>();
    if (num_levels < 0)
    {
      std::cerr << "The set number of levels (" << num_levels << ") should be a non-negative integer\n";
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

  if (vm.count("input-coefs"))
  {
    coefs_in = vm["input-coefs"].as<std::string>();
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
      std::cerr << "[ERROR] A mesh with boundaries is not supported by Butterfly wavelet transform.\n";
      return 1;
    }
  }

  std::vector<std::vector<Vector3>> coefs;

  if (coefs_in.empty())
  {
    load_coefs(coefs, std::cin);
  }
  else
  {
    std::ifstream coefs_in_scanner(coefs_in);
    if (!coefs_in_scanner.is_open())
    {
      std::cerr << "[ERROR] " << coefs_in << " file not found\n";
      return 1;
    }
    load_coefs(coefs, coefs_in_scanner);
    coefs_in_scanner.close();
  }

  if (auto_padding)
  {
    coefficients_padding(coefs, mesh, num_levels);
  }

  if (!auto_padding && !coefs_precheck(coefs, mesh, num_levels))
  {
    std::cerr << "[ERROR] Invalid wavelet coefficients\n";
    return 1;
  }

  if (method == "Butterfly")
  {
    wtlib::butterfly_synthesize(mesh, coefs, num_levels);
  }
  else
  {
    wtlib::loop_synthesize(mesh, coefs, num_levels);
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
      std::cerr << "[ERROR] Fail to open file " << mesh_out << " to write\n";
      return 1;
    }
    mesh_out_file << mesh;
    mesh_out_file.close();
  }

  return 0;
}
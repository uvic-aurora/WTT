#include <wtlib/butterfly_wavelet_transform.hpp>
#include <wtlib/loop_wavelet_transform.hpp>
#include <wtlib/mesh_types.hpp>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <chrono>
#include <fstream>

namespace po = boost::program_options;

using Vertex_handle = typename Mesh::Vertex_handle;
using Vector3 = typename Mesh::Traits::Vector_3;

void apply_hard_thresholding(std::vector<std::vector<Vector3>>& coefs, double threshold)
{
  if (threshold < 0)
  {
    std::cerr << "0 coefficients are set to zero\n";
    return;
  }

  int total = 0;

  int i = 0;
  for (std::vector<Vector3>& band_coefs : coefs)
  {
    total += band_coefs.size();
    for (Vector3& v : band_coefs)
    {
      if (v.squared_length() < threshold * threshold)
      {
        v = Vector3(0.0, 0.0, 0.0);
        ++i;
      }
    }
  }
  std::cerr << i << " out of " << total << " coefficients are set to zero\n";
}

void apply_lowpass_filter(std::vector<std::vector<Vector3>>& coefs)
{
  for (std::vector<Vector3>& band_coefs : coefs)
  {
    for (Vector3& v : band_coefs)
    {
      v = Vector3(0.0, 0.0, 0.0);
    }
  }
}


void apply_compressing(std::vector<std::vector<Vector3>>& coefs, double compression)
{
  std::vector<std::pair<int, int>> indexes;
  for (int b = 0; b < coefs.size(); ++b)
  {
    for (int i = 0; i < coefs[b].size(); ++i)
    {
      indexes.emplace_back(std::pair<int, int>{b, i});
    }
  }

  // Sort coefs based on their L2 norm
  auto compare = [&coefs](const std::pair<int, int>& lhs,
                          const std::pair<int, int>& rhs)
                          {
                            const Vector3& lv = coefs[lhs.first][lhs.second];
                            const Vector3& rv = coefs[rhs.first][rhs.second];
                            return lv.squared_length() > rv.squared_length();
                          };
  std::sort(indexes.begin(), indexes.end(), compare);

  int desired_length = int(indexes.size() * compression);

  if (desired_length >= indexes.size())
  {
    return;
  }

  // Drop the smaller coefficients
  for (int i = desired_length; i < indexes.size(); ++i)
  {
    std::pair<int, int> index = indexes[i];
    coefs[index.first][index.second] = Vector3 {0.0, 0.0, 0.0};
  }

  std::cerr << "Dropped " << indexes.size() - desired_length << " out of " << indexes.size() << " coefficients\n";
}

int main(int argc, char** argv)
{
  po::options_description descriptions(R"(A program performs the Loop or Butterfly wavelet filtering on a triangle mesh.

Usage:
    wtl_wavelet_analyze -m <scheme> -l <level> 
                        [--input-mesh <args>] [--output-mesh <args>]
                        (-t <args> | -L | -c <args>)

These are accepted options)");
  descriptions.add_options()
    ("help,h", "Display usage.")
    ("method,m", po::value<std::string>(), "Select a wavelet transform scheme:\n"
                                           "\t - Butterfly\n"
                                           "\t - Loop.")
    ("level,l", po::value<int>(), "Set the number of wavelet transform levels.")
    ("input-mesh,i", po::value<std::string>(), "Set the file path for the input mesh. "
                                               "Without this option, program will read input mesh from standard input.")
    ("output-mesh,o", po::value<std::string>(), "Set the file path for the output mesh. "
                                                "Without this option, program will output the mesh to standard output.")
    ("threshold,t", po::value<double>(), "Set the filtering scheme to hard-thresholding. "
                                         "Any wavelet coefficients whose L2 norms are less than the given threshold will be set to zero.")
    ("lowpass-filter,L", "Set the filtering scheme to lowpass. All the wavelet coefficients will be set to zero. ")
    ("compress,c", po::value<double>(), "Set the filtering scheme to compression. "
                                        "The wavelet coefficients whose magnitudes are in the top given percentage will be preserved, and the others are set to zero.");


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

  if (!unknown_opts.empty())
  {
    std::cerr << "Unknown option: " << unknown_opts[0] << '\n';
    return 1;
  }

  
  int num_levels = 0;
  std::string mesh_out;
  std::string mesh_in;
  std::string method;
  double threshold = 0.0;
  double compress = 100;
  bool hard_thresholding = false;
  bool lowpass_filtering = false;
  bool compression = false;

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
      std::cerr << method << " is not supported, please select one from below: "
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
      std::cerr << "The set number of level (" << num_levels << ") should be a non-negative integer\n";
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

  if (vm.count("threshold"))
  {
    hard_thresholding = true;
    lowpass_filtering = false;
    compression = false;
    threshold = vm["threshold"].as<double>();
  }
  
  if (vm.count("compress"))
  {
    compression = true;
    hard_thresholding = false;
    lowpass_filtering = false;
    compress = vm["compress"].as<double>();
    if (compress < 0 || compress > 100)
    {
      std::cerr << "The set compressing percentage (" << compress << ") is not in 0-100\n";
      return 1;
    }
    compress /= 100.0;
  }

  if (vm.count("lowpass-filter"))
  {
    lowpass_filtering = true;
    hard_thresholding = false;
    compression = false;
  }



  // Load mesh.
  Mesh mesh;

  // Validate mesh.
  if (mesh_in.empty())
  {
    if (!(std::cin >> mesh))
    {
      std::cerr << "[ERROR] Fail to read mesh from stdin\n";
      return 1;
    }
  }
  else
  {
    std::ifstream mesh_in_file(mesh_in);
    if (!(mesh_in_file) || !(mesh_in_file >> mesh))
    {
      std::cerr << "[ERROR] Fail to read mesh from " << mesh_in << '\n';
      return 1;
    }
    mesh_in_file.close();
  }

  std::vector<std::vector<Vector3>> coefs;

  if (method == "Butterfly")
  {
    if (!mesh.is_closed())
    {
      std::cerr << "[ERROR] A mesh with boundaries is not supported by the Butterfly wavelet transform\n";
      return 1;
    }
  
    if (!wtlib::butterfly_analyze(mesh, coefs, num_levels))
    {
      std::cerr << "[ERROR] The input mesh does not have " << num_levels 
                << " levels of subdivision connectivity\n";
      return 1;
    }
  }
  else
  {
    if (!wtlib::loop_analyze(mesh, coefs, num_levels))
    {
      std::cerr << "[ERROR] The input mesh does not have " << num_levels 
                << " levels of subdivision connectivity\n";
      return 1;
    }
  }

  if (lowpass_filtering)
  {
    apply_lowpass_filter(coefs);
  }
  else if (compression)
  {
    apply_compressing(coefs, compress);
  }
  else
  {
    apply_hard_thresholding(coefs, threshold);
  }

  if (method == "Butterfly")
  {
    wtlib::butterfly_synthesize(mesh, coefs, num_levels);
  }
  else
  {
    wtlib::loop_synthesize(mesh, coefs, num_levels);
  }

  // Output mesh
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
#include <boost/program_options.hpp>

#include <CGAL/IO/File_scanner_OFF.h>
#include <CGAL/Simple_cartesian.h>

#include <fstream>
#include <iostream>

namespace po = boost::program_options;

using Faces = std::vector<std::vector<int>>;

using Point = CGAL::Simple_cartesian<double>::Point_3;
using Points = std::vector<Point>;

bool read_OFF( std::istream& in,
          Points& points,
          Faces& polygons,
          bool /* verbose */ = false)
{
  CGAL::File_scanner_OFF scanner(in);

  points.resize(scanner.size_of_vertices());
  polygons.resize(scanner.size_of_facets());
  for (std::size_t i = 0; i < scanner.size_of_vertices(); ++i) {
    double x, y, z, w;
    scanner.scan_vertex( x, y, z, w);
    CGAL_assertion(w!=0);
    points[i] = Point(x/w, y/w, z/w);
    scanner.skip_to_next_vertex( i);
  }
  if(!in)
    return false;

  for (std::size_t i = 0; i < scanner.size_of_facets(); ++i) {
    std::size_t no;

    scanner.scan_facet( no, i);
    polygons[i].resize(no);
    for(std::size_t j = 0; j < no; ++j) {
      std::size_t id;
      scanner.scan_facet_vertex_index(id, i);
      if(id < scanner.size_of_vertices())
      {
        polygons[i][j] = id;
      }
      else
        return false;
    }
  }
  return in.good();
}
int main(int argc, char** argv)
{
  po::options_description d("Calculate L2 error");

  d.add_options()
    ("help,h", "Usage\n")
    ("input,i", po::value<std::vector<std::string>>()->multitoken(), "Input files");

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(d).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);

  std::vector<std::string> unknown_opts =
      po::collect_unrecognized(parsed.options, po::include_positional);
  
  if (!unknown_opts.empty())
  {
    std::cerr << "Unknown option: " << unknown_opts[0] << '\n';
    return 1;
  }

  std::string f0;
  std::string f1;

  if (vm.count("input"))
  {
    std::vector<std::string> files = vm["input"].as<std::vector<std::string>>();
    if (files.size() < 2)
    {
      std::cerr << "Need two files to compare\n";
      return 1;
    }
    f0 = files[0];
    f1 = files[1];
  }
  else
  {
    std::cerr << "Please set input files path\n";
    return 1;
  }

  std::ifstream fs0(f0);
  std::ifstream fs1(f1);

  if (!fs0.is_open())
  {
    std::cerr << "Unable to open file " << f0 << "\n";
    return 1;
  }
  if (!fs1.is_open())
  {
    std::cerr << "Unable to open file " << f1 << "\n";
    return 1;
  }


  Points ps0;
  Points ps1;
  Faces facets0;
  Faces facets1;

  if (!read_OFF(fs0, ps0, facets0))
  {
    std::cerr << "Fail to parse " << f0 << '\n';
    return 1;
  }
  if (!read_OFF(fs1, ps1, facets1))
  {
    std::cerr << "Fail to parse " << f1 << '\n';
    return 1;
  }

  fs0.close();
  fs1.close();

  if (ps0.size() != ps1.size())
  {
    std::cerr << "Points size mismatch\n";
    return 1;
  }

  // Calculate L2 errors
  double el2 = 0.0;

  double min_e = std::numeric_limits<double>::max();
  double max_e = std::numeric_limits<double>::min();

  for (int i = 0; i < ps0.size(); ++i)
  {
    const Point& p0 = ps0[i];
    const Point& p1 = ps1[i];
    double dx = p1[0] - p0[0];
    double dy = p1[1] - p0[1];
    double dz = p1[2] - p0[2];
    double e = std::sqrt(dx * dx + dy * dy + dz * dz);
    el2 += e;

    if (e < min_e)
    {
      min_e = e;
    }
    if (e > max_e)
    {
      max_e = e;
    }
  }

  std::cout << "Number of vertices: " << ps0.size() << "\n"
            << "L2 error total: " << el2 << "\n"
            << "L2 error mean: " << el2 / ps0.size() << "\n"
            << "L2 error max: " << max_e << "\n"
            << "L2 error min: " << min_e << "\n";
}
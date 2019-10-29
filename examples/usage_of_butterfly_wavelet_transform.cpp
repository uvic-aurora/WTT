// Required headers
#include <wtlib/butterfly_wavelet_transform.hpp>

#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>

using Kernel = CGAL::Simple_cartesian<double>;

// The type used to represent wavelet coefficients.
using Vector3 = Kernel::Vector_3;

// The type of a mesh.
using Mesh = CGAL::Polyhedron_3<Kernel>;

int main() {
  // Initialize a mesh and read data from stdin.
  Mesh mesh;
  std::cin >> mesh;

  // Only pure triangle mesh is supported.
  if (!mesh.is_pure_triangle()) {
    std::cerr << "The input mesh is not a pure triangle mesh.\n";
    return 1;
  }

  // Construct a vector of vector to store the computed wavelet coefficients.
  // Each internal vector buffers the wavelet coefficients of a resolution
  // level. For example, coefs[0] are wavelet coefficients at resolution 1, and
  // coefs[1] are wavelet coefficients at resolution 2, and so forth.
  std::vector<std::vector<Vector3>> coefs;

  // Compute 3-level Butterfly forward wavelet transform.
  bool res = wtlib::butterfly_analyze(mesh, coefs, 3);
  if (res) {
    // Compute 3-level Butterfly inverse wavelet transform.
    wtlib::butterfly_synthesize(mesh, coefs, 3);
  } else {
    // The forward wavelet transform can fail since the input mesh may not have
    // enough levels of subdivision connectivity.
    std::cerr << "The input mesh does not have 3 levels of subdivision connectivity.\n";
    return 1;
  }
  return 0;
}
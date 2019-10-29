#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <test_utils.hpp>

#include <wtlib/ptq_impl/loop_wavelet_operations.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/Polyhedron_iostream.h>


#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif


using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Vertex_handle = typename Mesh::Vertex_handle;
using Point = typename Mesh::Traits::Point_3;

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
using Utils = Wtlib_test_helper<Mesh, Mesh_ops>;

using Classify = wtlib::ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
using Loop = wtlib::ptq_impl::Loop_lift_operations<Mesh, Mesh_ops, true>;
using Loop_analysis = wtlib::ptq_impl::Loop_analysis_operations<Mesh, Mesh_ops>;


TEST_CASE("Check border_edges_to_border_olds",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open3_7_19.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};
  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);


  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 19);
  REQUIRE(bands.size() == 3);

  Loop::border_edges_to_border_olds(m, m_ops, bands[0], bands[1], bands[2]);

  std::vector<Point> res_from_py 
  {
      Point{ 0.0000000000,  0.0000000000,  1.0000000000},
      Point{ 0.7555090755, -0.0049620367,  0.2458717675},
      Point{-0.3778395212,  0.6084786040,  0.2344988802},
      Point{-0.3831101916, -0.6658132397,  0.2578882051},
      Point{ 2.2336376312,  3.6826670858, -0.8659255211},
      Point{-4.3036929220, -0.0008252757, -0.6818687308},
      Point{ 2.0646146535, -3.7441384826, -0.7139468952},
      Point{ 0.5341897946,  0.0026782580,  0.5347758624},
      Point{-0.2094556174,  0.4206216202,  0.4379279355},
      Point{-0.2781079822, -0.4371948754,  0.4865335356},
      Point{ 0.2986009524,  0.3876981515, -0.0973978911},
      Point{-0.4489899767,  0.1230674696,  0.0722269621},
      Point{ 0.1320965559, -0.5056105374,  0.0882451269},
      Point{ 1.1480791824,  1.2136157884, -0.1280601788},
      Point{-1.6289665128,  0.4337895217, -0.3289089815},
      Point{ 0.5298680264, -1.7003361654, -0.2084228534},
      Point{ 0.3846455552,  1.8133548855, -0.1400887790},
      Point{-1.7636476432, -0.4321389704, -0.3073535569},
      Point{ 1.3409026666, -1.2036917150, -0.3636833562},
  };

  for (Vertex_handle v : vertices)
  {
    INFO("  v " << m_ops.get_vertex_id(v));
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check border_olds_to_border_edges",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open4_9_25.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 25);;
  REQUIRE(bands.size() == 3);;

  Loop::border_olds_to_border_edges(m, m_ops, bands[0], bands[1], bands[2]);

  std::vector<Point> res_from_py 
  {
    Point { 0.0000000000,  0.0000000000,  1.0000000000},
    Point { 1.0000000000,  0.0000000000,  0.0000000000},
    Point { 0.0000000000,  1.0000000000,  0.0000000000},
    Point {-1.0000000000,  0.0000000000,  0.0000000000},
    Point {-0.0000000000, -1.0000000000,  0.0000000000},
    Point { 1.4142135624,  1.4142135624, -0.5000000000},
    Point {-1.4142135624,  1.4142135624, -0.5000000000},
    Point {-1.4142135624, -1.4142135624, -0.5000000000},
    Point { 1.4142135624, -1.4142135624, -0.5000000000},
    Point { 0.4446281802,  0.0415751947,  0.4294626234},
    Point {-0.0257811686,  0.6237329921,  0.4302420203},
    Point {-0.3977862011, -0.0498577253,  0.4177386155},
    Point {-0.0206371299, -0.4301702207,  0.5926144526},
    Point { 0.5808349371,  0.5624509946, -0.0876117222},
    Point {-0.4198374457,  0.4534017640, -0.0800284114},
    Point {-0.4118887958, -0.5769923066,  0.0970919011},
    Point { 0.5118377333, -0.5357420899, -0.0054523517},
    Point { 0.0391604912, -0.0328880802, -0.0932659983},
    Point { 0.0088297084,  0.0426212427,  0.1079126234},
    Point {-0.1222699264, -0.0116576605, -0.0153059262},
    Point {-0.0043736002, -0.0972905894,  0.0129542365},
    Point { 0.0334433596, -0.0021752245, -0.0234914631},
    Point {-0.0570786251, -0.0092440921,  0.1065161026},
    Point {-0.0589914033,  0.0682435846,  0.0396330163},
    Point { 0.0645080487,  0.1055479420, -0.1205863316},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check inner_edges_to_inner_olds",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open4_9_25.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 25);;
  REQUIRE(bands.size() == 3);;

  Loop::inner_edges_to_inner_olds(m, m_ops, bands[0], bands[1], bands[2]);

  std::vector<Point> res_from_py 
  {
    Point {-0.0003648361, -0.1595468740,  2.8341169704},
    Point { 1.0000000000,  0.0000000000,  0.0000000000},
    Point { 0.0000000000,  1.0000000000,  0.0000000000},
    Point {-1.0000000000,  0.0000000000,  0.0000000000},
    Point {-0.0000000000, -1.0000000000,  0.0000000000},
    Point { 1.4142135624,  1.4142135624, -0.5000000000},
    Point {-1.4142135624,  1.4142135624, -0.5000000000},
    Point {-1.4142135624, -1.4142135624, -0.5000000000},
    Point { 1.4142135624, -1.4142135624, -0.5000000000},
    Point { 0.4446281802,  0.0415751947,  0.4294626234},
    Point {-0.0257811686,  0.6237329921,  0.4302420203},
    Point {-0.3977862011, -0.0498577253,  0.4177386155},
    Point {-0.0206371299, -0.4301702207,  0.5926144526},
    Point { 0.5808349371,  0.5624509946, -0.0876117222},
    Point {-0.4198374457,  0.4534017640, -0.0800284114},
    Point {-0.4118887958, -0.5769923066,  0.0970919011},
    Point { 0.5118377333, -0.5357420899, -0.0054523517},
    Point { 1.2462672724,  0.6742187010, -0.3432659983},
    Point {-0.6982770728,  1.2497280239, -0.1420873766},
    Point {-1.3293767076, -0.7187644417, -0.2653059262},
    Point { 0.7027331810, -1.3043973706, -0.2370457635},
    Point { 0.7405501408,  1.2049315567, -0.2734914631},
    Point {-1.2641854063,  0.6978626891, -0.1434838974},
    Point {-0.7660981845, -1.1388631966, -0.2103669837},
    Point { 1.2716148299, -0.6015588392, -0.3705863316},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

TEST_CASE("Check inner_olds_to_inner_edges",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open4_9_25.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));
  REQUIRE(vertices.size() == 25);;
  REQUIRE(bands.size() == 3);;

  Loop::inner_olds_to_inner_edges(m, m_ops, bands[0], bands[1], bands[2]);

  std::vector<Point> res_from_py 
  {
    Point { 0.0000000000,  0.0000000000,  1.0000000000},
    Point { 1.0000000000,  0.0000000000,  0.0000000000},
    Point { 0.0000000000,  1.0000000000,  0.0000000000},
    Point {-1.0000000000,  0.0000000000,  0.0000000000},
    Point {-0.0000000000, -1.0000000000,  0.0000000000},
    Point { 1.4142135624,  1.4142135624, -0.5000000000},
    Point {-1.4142135624,  1.4142135624, -0.5000000000},
    Point {-1.4142135624, -1.4142135624, -0.5000000000},
    Point { 1.4142135624, -1.4142135624, -0.5000000000},
    Point { 0.0696281802,  0.0415751947,  0.0544626234},
    Point {-0.0257811686,  0.2487329921,  0.0552420203},
    Point {-0.0227862011, -0.0498577253,  0.0427386155},
    Point {-0.0206371299, -0.0551702207,  0.2176144526},
    Point { 0.0290582418,  0.0106742993, -0.1501117222},
    Point { 0.1319392496, -0.0983749313, -0.1425284114},
    Point { 0.1398878995, -0.0252156113,  0.0345919011},
    Point {-0.0399389620,  0.0160346054, -0.0679523517},
    Point { 1.2462672724,  0.6742187010, -0.3432659983},
    Point {-0.6982770728,  1.2497280239, -0.1420873766},
    Point {-1.3293767076, -0.7187644417, -0.2653059262},
    Point { 0.7027331810, -1.3043973706, -0.2370457635},
    Point { 0.7405501408,  1.2049315567, -0.2734914631},
    Point {-1.2641854063,  0.6978626891, -0.1434838974},
    Point {-0.7660981845, -1.1388631966, -0.2103669837},
    Point { 1.2716148299, -0.6015588392, -0.3705863316},
  };


  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check border_edges_to_border_olds_dual",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open4_9_25.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 25);;
  REQUIRE(bands.size() == 3);;

  Loop::border_edges_to_border_olds_dual(m, m_ops, bands[0], bands[1], bands[2]);

  std::vector<Point> res_from_py 
  {
    Point { 0.0000000000,  0.0000000000,  1.0000000000},
    Point { 2.0498554210,  0.0569766437, -0.2784860752},
    Point { 0.0255953102,  2.0301043612, -0.1262896961},
    Point {-2.0856250455, -0.0319414343, -0.1481141698},
    Point {-0.0223669948, -2.0339018513, -0.1148151456},
    Point { 2.3495604519,  2.2788507908, -0.7270747051},
    Point {-2.3338374900,  2.3454344331, -0.5481515260},
    Point {-2.4088893110, -2.2754159362, -0.6779421219},
    Point { 2.3606250399, -2.3276315683, -0.7145367337},
    Point { 0.4446281802,  0.0415751947,  0.4294626234},
    Point {-0.0257811686,  0.6237329921,  0.4302420203},
    Point {-0.3977862011, -0.0498577253,  0.4177386155},
    Point {-0.0206371299, -0.4301702207,  0.5926144526},
    Point { 0.5808349371,  0.5624509946, -0.0876117222},
    Point {-0.4198374457,  0.4534017640, -0.0800284114},
    Point {-0.4118887958, -0.5769923066,  0.0970919011},
    Point { 0.5118377333, -0.5357420899, -0.0054523517},
    Point { 1.2462672724,  0.6742187010, -0.3432659983},
    Point {-0.6982770728,  1.2497280239, -0.1420873766},
    Point {-1.3293767076, -0.7187644417, -0.2653059262},
    Point { 0.7027331810, -1.3043973706, -0.2370457635},
    Point { 0.7405501408,  1.2049315567, -0.2734914631},
    Point {-1.2641854063,  0.6978626891, -0.1434838974},
    Point {-0.7660981845, -1.1388631966, -0.2103669837},
    Point { 1.2716148299, -0.6015588392, -0.3705863316},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

TEST_CASE("Check inner_edges_to_inner_olds_dual",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open4_9_25.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 25);;
  REQUIRE(bands.size() == 3);;

  Loop::inner_edges_to_inner_olds_dual(m, m_ops, bands[0], bands[1], bands[2]);

  std::vector<Point> res_from_py 
  {
    Point {-0.1460992289,  0.1587074347,  2.0961068328},
    Point { 2.0144995603, -0.0055077540, -0.1363212407},
    Point { 0.1057645382,  2.0103592113, -0.1566985113},
    Point {-1.7759600307, -0.1544335865, -0.0551011109},
    Point { 0.0602603495, -2.0133554927,  0.1018570998},
    Point { 0.6608865184,  0.6847299895, -0.3863699900},
    Point {-0.8696958660,  0.8261638622, -0.3962053369},
    Point {-0.8800050478, -0.6658703052, -0.6259255430},
    Point { 0.7503740066, -0.7193707092, -0.4929284488},
    Point { 0.4446281802,  0.0415751947,  0.4294626234},
    Point {-0.0257811686,  0.6237329921,  0.4302420203},
    Point {-0.3977862011, -0.0498577253,  0.4177386155},
    Point {-0.0206371299, -0.4301702207,  0.5926144526},
    Point { 0.5808349371,  0.5624509946, -0.0876117222},
    Point {-0.4198374457,  0.4534017640, -0.0800284114},
    Point {-0.4118887958, -0.5769923066,  0.0970919011},
    Point { 0.5118377333, -0.5357420899, -0.0054523517},
    Point { 1.2462672724,  0.6742187010, -0.3432659983},
    Point {-0.6982770728,  1.2497280239, -0.1420873766},
    Point {-1.3293767076, -0.7187644417, -0.2653059262},
    Point { 0.7027331810, -1.3043973706, -0.2370457635},
    Point { 0.7405501408,  1.2049315567, -0.2734914631},
    Point {-1.2641854063,  0.6978626891, -0.1434838974},
    Point {-0.7660981845, -1.1388631966, -0.2103669837},
    Point { 1.2716148299, -0.6015588392, -0.3705863316},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check analysis on loop_test_open3_7_19",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open3_7_19.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 19);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point { 0.0207113643, -0.0440965303,  3.3804306702},
    Point { 0.4374970787, -0.1287987986, -1.0005244570},
    Point {-0.0843440605,  0.6072917144, -1.0624169645},
    Point {-0.3386637974, -0.3368852703, -0.8282796515},
    Point { 1.8826408044,  3.7191647093,  1.3472221395},
    Point {-4.3536835308, -0.3338407738,  0.8339308107},
    Point { 2.3845097315, -3.4638451315,  0.8341538026},
    Point { 0.3984470748, -0.0039260206, -1.7273324359},
    Point {-0.0618611881,  0.2606571814, -1.8213371410},
    Point {-0.1291958853, -0.2785863533, -1.7785788721},
    Point {-0.1047450112, -0.3041645709, -0.8720821938},
    Point { 0.3918126040,  0.1394604935, -0.7299706035},
    Point {-0.2481450341,  0.2087368776, -0.7142075009},
    Point {-0.3464941709, -0.6252367362,  0.1819666980},
    Point { 0.7117997088,  0.1299628575, -0.1052240562},
    Point {-0.3108842045,  0.5046396957,  0.0196064916},
    Point {-0.5432534998, -0.3322179594,  0.1756245414},
    Point { 0.5797539136, -0.0988197127, -0.0953632941},
    Point {-0.0691591979,  0.6708585446, -0.1296457923},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

TEST_CASE("Check analysis on open4_9_25.off",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open4_9_25.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 25);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point {-0.1395514328,  0.0848665465,  2.1872441217},
    Point { 0.9127430158,  0.0829870375, -0.6486769946},
    Point { 0.1341744561,  0.8937800453, -0.4662211591},
    Point {-0.7311406223, -0.1505952072, -0.3391500725},
    Point { 0.0087689813, -0.8294739860, -0.2239889203},
    Point { 1.5790136340,  1.5444395706, -0.0615267046},
    Point {-1.7768093678,  1.7465038579, -0.0370094405},
    Point {-1.8087092419, -1.5947148082, -0.3403448503},
    Point { 1.7060422261, -1.6989798387, -0.1439705160},
    Point { 0.1655496419,  0.1157414349, -0.8211155265},
    Point {-0.0224481555,  0.3970466230, -0.7806380106},
    Point {-0.1352605299,  0.0067657122, -0.7756403145},
    Point {-0.0371112553, -0.0752166175, -0.6242344360},
    Point { 0.0815323338,  0.0701602498, -0.5671920291},
    Point { 0.0827409484, -0.0521542550, -0.4817103617},
    Point { 0.0625691840, -0.0316277191, -0.3224402591},
    Point {-0.0080513461,  0.0241447646, -0.4904311809},
    Point {-0.0417714111, -0.2520423315, -0.1759184461},
    Point { 0.2358891368, -0.0639229651,  0.1826250950},
    Point {-0.0874223967,  0.2258167730,  0.0135783904},
    Point {-0.2337346296,  0.0225119975, -0.0008069741},
    Point {-0.1663908013, -0.1258295461, -0.0315755384},
    Point { 0.0110220079, -0.2346786332,  0.1829258282},
    Point { 0.1084054040,  0.2001283144,  0.0588616020},
    Point {-0.0195412042,  0.3543306362, -0.2009574378},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

TEST_CASE("Check analysis on open5_11_31",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open5_11_31.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 31);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point {-0.0289941780, -0.0772896754,  1.9006866550},
    Point { 0.8004928941,  0.1753177494, -0.2237778891},
    Point { 0.1400323522,  0.6885956878, -0.3560785317},
    Point {-0.6751724837,  0.6232633954, -0.4394455740},
    Point {-0.5270837097, -0.4247638589, -0.2920630923},
    Point { 0.2895876067, -0.5545967610, -0.2360151247},
    Point { 1.9778421266,  1.2889270567, -0.2528673997},
    Point {-0.6220283006,  2.2939209871, -0.3604006873},
    Point {-2.3713614251, -0.0802739495, -0.2565374198},
    Point {-0.8895501323, -2.3749767443, -0.3904438436},
    Point { 1.9585754079, -1.4273687302, -0.3697631177},
    Point { 0.2421071967,  0.0855428642, -0.5050230764},
    Point {-0.0237758825,  0.1292877583, -0.5540360248},
    Point {-0.2783926226,  0.2379167860, -0.5343473026},
    Point {-0.2329039937, -0.2063600677, -0.3519649744},
    Point {-0.0577295037, -0.1455945109, -0.3245315447},
    Point {-0.0325493836,  0.1488829592, -0.4056292427},
    Point {-0.0586679435, -0.0101361774, -0.4578262194},
    Point {-0.0129743042,  0.1135481090, -0.4274960230},
    Point { 0.1025425044,  0.0706132233, -0.4299490078},
    Point { 0.0006801979,  0.0177223890, -0.3062979610},
    Point { 0.0692751502, -0.0177141553,  0.1787432946},
    Point { 0.2763614306, -0.1587590570, -0.1346401572},
    Point { 0.1354154624,  0.2260450715, -0.0676881505},
    Point {-0.1423872824,  0.0813089810, -0.1267636473},
    Point {-0.0929212195,  0.1466164288,  0.1121961637},
    Point {-0.1992220575, -0.0077226921,  0.1382474662},
    Point { 0.0665143126, -0.0720067238, -0.2039134085},
    Point { 0.2824373251, -0.0500009606,  0.0524205901},
    Point { 0.2034122892,  0.2752311796, -0.1642241555},
    Point { 0.0092398204,  0.2421632188,  0.1513511190},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

TEST_CASE("Check analysis on open6_13_37",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_open6_13_37.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 37);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point {-0.0038803892, -0.0298923499,  1.6938705995},
    Point { 0.7128103924,  0.0609650122, -0.1363938214},
    Point { 0.5046667849,  0.6003312386, -0.1280164625},
    Point {-0.2484734676,  0.6250175648, -0.2878296389},
    Point {-0.7697860670, -0.0924016543, -0.2249077048},
    Point {-0.4093750208, -0.7254157237, -0.0604538691},
    Point { 0.4933809596, -0.6409052252, -0.2367211024},
    Point { 2.1002761575,  1.1445266379, -0.4801198317},
    Point {-0.1489406426,  2.3555599471, -0.4755396830},
    Point {-2.0511391734,  1.1549869413, -0.3661436075},
    Point {-2.0303054088, -1.0690694751, -0.4438185499},
    Point {-0.0315614637, -2.3078846957, -0.4498301099},
    Point { 2.0029768466, -1.2750636306, -0.3884611885},
    Point { 0.2365980734, -0.1383454573, -0.4124834193},
    Point { 0.1173470626,  0.1142716395, -0.3604878506},
    Point {-0.0380679142,  0.1517860777, -0.3422038906},
    Point {-0.1063755029,  0.0376891110, -0.3089382883},
    Point {-0.0749781170, -0.2396399107, -0.1849316608},
    Point { 0.1594708042, -0.1551648226, -0.4003825493},
    Point { 0.0567440989,  0.1062708519, -0.2407564741},
    Point { 0.1241800234,  0.0145754639, -0.3430377868},
    Point {-0.0144195312,  0.0621178081, -0.4239297735},
    Point {-0.1135319566, -0.1811513528, -0.2606546753},
    Point { 0.0793207088, -0.0893139435, -0.3321796506},
    Point { 0.0561033880,  0.0198344058, -0.3663504716},
    Point {-0.0086185441, -0.0678458554,  0.1793990918},
    Point { 0.1996538886, -0.3201516900,  0.0039198883},
    Point { 0.3066732687, -0.0355303828, -0.0088481703},
    Point {-0.0174303697,  0.0339502147,  0.1077977686},
    Point {-0.1153802185,  0.2654569236,  0.0595687735},
    Point {-0.1340099455, -0.1162625229,  0.1088592079},
    Point { 0.0079506848, -0.0879503157,  0.0686513355},
    Point {-0.1497554034, -0.2827043226, -0.1725252989},
    Point {-0.0047773790, -0.2604577768, -0.0882212709},
    Point { 0.0482443175, -0.0042083158,  0.1399237425},
    Point { 0.2429477505,  0.0879086687,  0.0442053979},
    Point {-0.1620133429,  0.0858273567,  0.1021807062},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

TEST_CASE("Check analysis on close3_5_14",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_close3_5_14.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 14);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point {-0.4599613329, -0.6622766098, -1.4034464747},
    Point { 0.0166675097, -0.3112089019,  1.2583216227},
    Point { 2.2635331583, -0.0904283914,  0.1455281298},
    Point {-0.7535048396,  1.8667631321, -0.0563903037},
    Point {-1.0122098006, -1.5096341964,  0.0976963832},
    Point {-0.6280966220,  0.1548146384, -1.7519303945},
    Point { 0.1128277860, -0.3385073893, -1.7119439733},
    Point {-0.0575822129,  0.6403785750, -1.8310872741},
    Point {-0.4736773003, -0.7101743953, -0.0646857439},
    Point { 1.0274054523,  0.3454825171, -0.0216805181},
    Point {-0.4843767854,  1.1044011750,  0.0921136507},
    Point {-0.4389754083,  0.2146635062,  1.7477895735},
    Point { 0.3287386306, -0.0349105079,  1.5945315025},
    Point { 0.2206013213,  0.7801649231,  1.7310213094},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check analysis on close4_6_18",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_close4_6_18.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 18);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point {-0.1172057526,  0.0329023067,  1.3260702493},
    Point {-0.0535265449,  0.0048258438, -1.3870633506},
    Point { 2.1463807805,  0.1818261770, -0.0685540848},
    Point {-0.1000581100,  1.9284451804,  0.0632760890},
    Point {-1.9031498932, -0.0905584411,  0.1268768398},
    Point { 0.0286766667, -2.0622049046, -0.0616112778},
    Point {-0.7367941703,  0.1003957363, -0.6247064642},
    Point {-0.0947496755, -0.8786798178, -0.4956949568},
    Point { 0.7460325988, -0.0762488022, -0.4435274074},
    Point {-0.0661436874,  0.6636577300, -0.6183327848},
    Point {-0.8142049507, -0.7415227813, -0.0518458540},
    Point { 0.7472147312, -1.0177154407,  0.1060188058},
    Point { 0.9617084315,  0.6395369517,  0.0059956715},
    Point {-0.6341237371,  0.8099796147, -0.0454787165},
    Point {-0.7851081891,  0.1096691966,  0.3497213559},
    Point {-0.0489539424, -0.9816716363,  0.6110048169},
    Point { 0.9211088536, -0.2444247597,  0.6604508847},
    Point {-0.0102071448,  0.8248077382,  0.3791762653},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check analysis on close5_7_22",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_close5_7_22.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 22);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point { 0.0511691245, -0.0422634914,  1.4547410846},
    Point {-0.0377826663,  0.0078355602, -1.4765957499},
    Point { 1.9873602101, -0.0237600109, -0.0075074485},
    Point { 0.5849648780,  1.9227555732,  0.0682918219},
    Point {-1.6595174252,  1.2357144588,  0.1261993918},
    Point {-1.6052361654, -1.1899724599,  0.0011891233},
    Point { 0.5702752971, -1.8868614409, -0.0416630792},
    Point {-0.8635952142, -0.0770203343, -0.2618490445},
    Point {-0.3062299484, -0.7920415645, -0.0421594775},
    Point { 0.7069806144, -0.4397702843, -0.0364482900},
    Point { 0.7095036528,  0.4821985388, -0.2126833528},
    Point {-0.4545156905,  0.7622764739, -0.3268168238},
    Point {-0.9451264755, -0.4706258420,  0.2167003261},
    Point { 0.1501277414, -1.0047399921,  0.2213691289},
    Point { 0.9156461053,  0.0887167787,  0.2681927206},
    Point { 0.2202716199,  1.0581368007,  0.0688228149},
    Point {-0.9638026327,  0.5240355278, -0.0485670955},
    Point {-1.0714621480, -0.0178338938,  0.3707105715},
    Point {-0.4222676415, -0.7362045341,  0.4459724599},
    Point { 0.5312050581, -0.3667779209,  0.5105825213},
    Point { 0.6029454050,  0.5276363698,  0.3122112699},
    Point {-0.3220891251,  0.7956811245,  0.2488732287},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}


TEST_CASE("Check analysis on close6_8_26",
          "[Loop analysis operations]")
{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_close6_8_26.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};

  // Initialize border mark
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

  REQUIRE(vertices.size() == 26);;
  REQUIRE(bands.size() == 3);;

  Loop_analysis::lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_py
  {
    Point {-0.0380677484,  0.0210841834,  1.4414537003},
    Point { 0.0137874785, -0.0982603951, -1.4666495487},
    Point { 1.9109553454,  0.0875605121,  0.0716905731},
    Point { 0.9576382878,  1.7451438142, -0.1380953131},
    Point {-1.1168645531,  1.6249530885,  0.0053338698},
    Point {-1.9804920353,  0.0871139068, -0.0525571501},
    Point {-1.0254446598, -1.8708148929, -0.0062050450},
    Point { 0.9896423290, -1.7119000376, -0.0096755352},
    Point {-1.1261989028,  0.0918600311, -0.0646185802},
    Point {-0.7365218816, -0.7089549896, -0.3902903972},
    Point { 0.2307952494, -0.9297386719, -0.2756939804},
    Point { 0.7875571205,  0.0734234045, -0.3061456610},
    Point { 0.3357555323,  0.5918173268, -0.2573005780},
    Point {-0.5442556975,  0.8521049111, -0.1610975402},
    Point {-1.0095250772, -0.3593957178, -0.1679820291},
    Point {-0.3156744377, -0.9350639940, -0.1693350071},
    Point { 0.5403422255, -0.4686415511, -0.1031086435},
    Point { 0.5785408966,  0.2457819996, -0.1731253311},
    Point {-0.1207686031,  0.6516838877,  0.0059442225},
    Point {-0.8707849071,  0.5607654916, -0.0028032386},
    Point {-1.1395667072,  0.0391487603,  0.1746068379},
    Point {-0.5488605025, -0.9375294190, -0.0523630329},
    Point { 0.2455719287, -0.9943947575,  0.0031460684},
    Point { 0.9300725752, -0.1020140729, -0.0009912847},
    Point { 0.4026776417,  0.5355881172,  0.1678661007},
    Point {-0.5981392514,  0.6365621421,  0.1925030089},
  };

  for (Vertex_handle v : vertices)
  {
    Point pc {res_from_py[m_ops.get_vertex_id(v)]};
    Point p = v->point();
    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}

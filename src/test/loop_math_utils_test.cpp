#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <wtlib/ptq_impl/loop_math_utils.hpp>


using LM = wtlib::ptq_impl::Loop_math;
using Eigen::Vector4d;

TEST_CASE("Check alpha", "[Loop math]")
{
  REQUIRE(LM::alpha(3) == Approx(0.4375000000 ));
  REQUIRE(LM::alpha(4) == Approx(0.5156250000 ));
  REQUIRE(LM::alpha(5) == Approx(0.5795339054 ));
  REQUIRE(LM::alpha(6) == Approx(0.6250000000 ));
  REQUIRE(LM::alpha(7) == Approx(0.6568255587 ));
  REQUIRE(LM::alpha(8) == Approx(0.6794575215 ));
  REQUIRE(LM::alpha(9) == Approx(0.6959348386 ));
  REQUIRE(LM::alpha(10) == Approx(0.7082224675 ));
  REQUIRE(LM::alpha(11) == Approx(0.7175917566 ));
  REQUIRE(LM::alpha(12) == Approx(0.7248797632 ));
}

TEST_CASE("Check beta", "[Loop math]")
{
  REQUIRE(LM::beta(3) == Approx(0.1000000000));
  REQUIRE(LM::beta(4) == Approx(0.2250000000));
  REQUIRE(LM::beta(5) == Approx(0.3272542486));
  REQUIRE(LM::beta(6) == Approx(0.4000000000));
  REQUIRE(LM::beta(7) == Approx(0.4509208939));
  REQUIRE(LM::beta(8) == Approx(0.4871320344));
  REQUIRE(LM::beta(9) == Approx(0.5134957418));
  REQUIRE(LM::beta(10) == Approx(0.5331559480));
  REQUIRE(LM::beta(11) == Approx(0.5481468105));
  REQUIRE(LM::beta(12) == Approx(0.5598076211));
}

TEST_CASE("Check gamma", "[Loop math]")
{
  REQUIRE(LM::gamma(3) == Approx(0.1875000000 ));
  REQUIRE(LM::gamma(4) == Approx(0.1210937500 ));
  REQUIRE(LM::gamma(5) == Approx(0.0840932189 ));
  REQUIRE(LM::gamma(6) == Approx(0.0625000000 ));
  REQUIRE(LM::gamma(7) == Approx(0.0490249202 ));
  REQUIRE(LM::gamma(8) == Approx(0.0400678098 ));
  REQUIRE(LM::gamma(9) == Approx(0.0337850179 ));
  REQUIRE(LM::gamma(10) == Approx(0.0291777532 ));
  REQUIRE(LM::gamma(11) == Approx(0.0256734767 ));
  REQUIRE(LM::gamma(12) == Approx(0.0229266864 ));
}

TEST_CASE("Check delta", "[Loop math]")
{
  REQUIRE(LM::delta(3) == Approx(0.3000000000 ));
  REQUIRE(LM::delta(4) == Approx(0.1937500000 ));
  REQUIRE(LM::delta(5) == Approx(0.1345491503 ));
  REQUIRE(LM::delta(6) == Approx(0.1000000000 ));
  REQUIRE(LM::delta(7) == Approx(0.0784398723 ));
  REQUIRE(LM::delta(8) == Approx(0.0641084957 ));
  REQUIRE(LM::delta(9) == Approx(0.0540560287 ));
  REQUIRE(LM::delta(10) == Approx(0.0466844052 ));
  REQUIRE(LM::delta(11) == Approx(0.0410775627 ));
  REQUIRE(LM::delta(12) == Approx(0.0366826982 ));
}

TEST_CASE("Check get_weight", "[Loop math]")
{
  Vector4d w, m;
  w = LM::get_weight(3, 6, 6, 6);
  m = LM::get_weight(6, 3, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.943533).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.192905).margin(1e-6));
  REQUIRE(w(2) == Approx(0.229306).margin(1e-6));
  REQUIRE(w(3) == Approx(0.229306).margin(1e-6));

  w = LM::get_weight(4, 6, 6, 6);
  m = LM::get_weight(6, 4, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.540222).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.248487).margin(1e-6));
  REQUIRE(w(2) == Approx(0.134022).margin(1e-6));
  REQUIRE(w(3) == Approx(0.134022).margin(1e-6));

  w = LM::get_weight(5, 6, 6, 6);
  m = LM::get_weight(6, 5, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.371945).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.272331).margin(1e-6));
  REQUIRE(w(2) == Approx(0.093146).margin(1e-6));
  REQUIRE(w(3) == Approx(0.093146).margin(1e-6));

  w = LM::get_weight(6, 6, 6, 6);
  m = LM::get_weight(6, 6, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.284905).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.284905).margin(1e-6));
  REQUIRE(w(2) == Approx(0.071591).margin(1e-6));
  REQUIRE(w(3) == Approx(0.071591).margin(1e-6));

  w = LM::get_weight(7, 6, 6, 6);
  m = LM::get_weight(6, 7, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.232761).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.292500).margin(1e-6));
  REQUIRE(w(2) == Approx(0.058571).margin(1e-6));
  REQUIRE(w(3) == Approx(0.058571).margin(1e-6));

  w = LM::get_weight(8, 6, 6, 6);
  m = LM::get_weight(6, 8, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.198097).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.297558).margin(1e-6));
  REQUIRE(w(2) == Approx(0.049901).margin(1e-6));
  REQUIRE(w(3) == Approx(0.049901).margin(1e-6));

  w = LM::get_weight(9, 6, 6, 6);
  m = LM::get_weight(6, 9, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.173274).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.301174).margin(1e-6));
  REQUIRE(w(2) == Approx(0.043701).margin(1e-6));
  REQUIRE(w(3) == Approx(0.043701).margin(1e-6));

  w = LM::get_weight(10, 6, 6, 6);
  m = LM::get_weight(6, 10, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.154510).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.303900).margin(1e-6));
  REQUIRE(w(2) == Approx(0.039029).margin(1e-6));
  REQUIRE(w(3) == Approx(0.039029).margin(1e-6));

  w = LM::get_weight(20, 6, 6, 6);
  m = LM::get_weight(6, 20, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.077147).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.314981).margin(1e-6));
  REQUIRE(w(2) == Approx(0.020032).margin(1e-6));
  REQUIRE(w(3) == Approx(0.020032).margin(1e-6));

  w = LM::get_weight(100, 6, 6, 6);
  m = LM::get_weight(6, 100, 6, 6);
  REQUIRE(w(0) == m(1));
  REQUIRE(w(1) == m(0));
  REQUIRE(w(2) == m(2));
  REQUIRE(w(3) == m(3));
  REQUIRE(w(0) == Approx(-0.016045).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.323481).margin(1e-6));
  REQUIRE(w(2) == Approx(0.005461).margin(1e-6));
  REQUIRE(w(3) == Approx(0.005461).margin(1e-6));



  w = LM::get_weight(6, 6, 3, 6);
  m = LM::get_weight(6, 6, 6, 3);
  REQUIRE(w(0) == Approx(m(0)).margin(1e-15));
  REQUIRE(w(1) == Approx(m(1)).margin(1e-15));
  REQUIRE(w(2) == Approx(m(3)).margin(1e-15));
  REQUIRE(w(3) == Approx(m(2)).margin(1e-15));
  REQUIRE(w(0) == Approx(-0.320177).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.320177).margin(1e-6));
  REQUIRE(w(2) == Approx(0.227782).margin(1e-6));
  REQUIRE(w(3) == Approx(0.090494).margin(1e-6));
  
  w = LM::get_weight(6, 6, 4, 6);
  m = LM::get_weight(6, 6, 6, 4); 
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.300018).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.300018).margin(1e-6));
  REQUIRE(w(2) == Approx(0.135809).margin(1e-6));
  REQUIRE(w(3) == Approx(0.079737).margin(1e-6));
  
  w = LM::get_weight(6, 6, 5, 6);
  m = LM::get_weight(6, 6, 6, 5);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.290306).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.290306).margin(1e-6));
  REQUIRE(w(2) == Approx(0.093943).margin(1e-6));
  REQUIRE(w(3) == Approx(0.074512).margin(1e-6));
  
  w = LM::get_weight(6, 6, 6, 6);
  m = LM::get_weight(6, 6, 6, 6);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.284905).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.284905).margin(1e-6));
  REQUIRE(w(2) == Approx(0.071591).margin(1e-6));
  REQUIRE(w(3) == Approx(0.071591).margin(1e-6));
  
  w = LM::get_weight(6, 6, 7, 6);
  m = LM::get_weight(6, 6, 6, 7);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.281566).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.281566).margin(1e-6));
  REQUIRE(w(2) == Approx(0.058082).margin(1e-6));
  REQUIRE(w(3) == Approx(0.069779).margin(1e-6));
  
  w = LM::get_weight(6, 6, 8, 6);
  m = LM::get_weight(6, 6, 6, 8);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.279321).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.279321).margin(1e-6));
  REQUIRE(w(2) == Approx(0.049109).margin(1e-6));
  REQUIRE(w(3) == Approx(0.068559).margin(1e-6));
  
  w = LM::get_weight(6, 6, 9, 6);
  m = LM::get_weight(6, 6, 6, 9);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.277711).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.277711).margin(1e-6));
  REQUIRE(w(2) == Approx(0.042714).margin(1e-6));
  REQUIRE(w(3) == Approx(0.067683).margin(1e-6));
  
  w = LM::get_weight(6, 6, 10, 6);
  m = LM::get_weight(6, 6, 6, 10);  
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.276497).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.276497).margin(1e-6));
  REQUIRE(w(2) == Approx(0.037910).margin(1e-6));
  REQUIRE(w(3) == Approx(0.067023).margin(1e-6));
  
  w = LM::get_weight(6, 6, 20, 6);
  m = LM::get_weight(6, 6, 6, 20);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.271609).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.271609).margin(1e-6));
  REQUIRE(w(2) == Approx(0.018528).margin(1e-6));
  REQUIRE(w(3) == Approx(0.064364).margin(1e-6));
  
  w = LM::get_weight(6, 6, 100, 6);
  m = LM::get_weight(6, 6, 6, 100);   
  REQUIRE(w(0) == Approx(m(0)));
  REQUIRE(w(1) == Approx(m(1)));
  REQUIRE(w(2) == Approx(m(3)));
  REQUIRE(w(3) == Approx(m(2)));
  REQUIRE(w(0) == Approx(-0.267949).margin(1e-6));
  REQUIRE(w(1) == Approx(-0.267949).margin(1e-6));
  REQUIRE(w(2) == Approx(0.003793).margin(1e-6));
  REQUIRE(w(3) == Approx(0.062377).margin(1e-6));
}

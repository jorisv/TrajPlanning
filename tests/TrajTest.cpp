// This file is part of PG.
//
// PG is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with PG.  If not, see <http://www.gnu.org/licenses/>.
// boost

// include
// std
#include <fstream>
#include <iostream>
#include <tuple>

// boost
#define BOOST_TEST_MODULE Algo test
#include <boost/test/unit_test.hpp>
#include <boost/math/constants/constants.hpp>

// RBDyn
#include <RBDyn/FK.h>
#include <RBDyn/FV.h>
#include <RBDyn/MultiBody.h>
#include <RBDyn/MultiBodyConfig.h>
#include <RBDyn/MultiBodyGraph.h>

// TrajPlanning
#include "ObsPen.h"

const Eigen::Vector3d gravity(0., 9.81, 0.);


/// @return An simple Z*6 arm with Y as up axis.
std::tuple<rbd::MultiBody, rbd::MultiBodyConfig> makeZ12Arm(bool isFixed=true)
{
  using namespace Eigen;
  using namespace sva;
  using namespace rbd;

  MultiBodyGraph mbg;

  double mass = 1.;
  Matrix3d I = Matrix3d::Identity();
  Vector3d h = Vector3d::Zero();

  RBInertiad rbi(mass, h, I);

  for(int i = 0; i < 7; ++i)
  {
    std::stringstream ss;
    ss << "b" << i;
    mbg.addBody({rbi, i, ss.str()});
  }

  for(int i = 0; i < 6; ++i)
  {
    std::stringstream ss;
    ss << "j" << i;
    mbg.addJoint({Joint::RevZ, true, i, ss.str()});
  }

  PTransformd to(Vector3d(0., 0.5, 0.));
  PTransformd from(Vector3d(0., 0., 0.));

  mbg.linkBodies(0, from, 1, from, 0);
  for(int i = 1; i < 6; ++i)
  {
    mbg.linkBodies(i, to, i + 1, from, i);
  }

  MultiBody mb = mbg.makeMultiBody(0, isFixed);

  MultiBodyConfig mbc(mb);
  mbc.zero(mb);

  return std::make_tuple(mb, mbc);
}


BOOST_AUTO_TEST_CASE(ObsPenTest)
{
  using namespace Eigen;
  using namespace sva;
  using namespace rbd;
  namespace cst = boost::math::constants;

  std::vector<double> pen =
  {
    0.19338496,  0.08683781,  0.76232272,  0.10032556,  0.0311701 ,
    0.74030221,  0.4986186 ,  0.58865215,  0.63947176,  0.37107554,
    0.77703448,  0.94472095,  0.16495522,  0.2537881 ,  0.12636114,
    0.89338157,  0.18361576,  0.7980018 ,  0.58179607,  0.19751129,
    0.2025195 ,  0.70539315,  0.06764872,  0.90060331,  0.23950046,
    0.30006224,  0.33118872
  };
  std::vector<double> penGradX =
  {
    0.17769058,  0.69019667,  0.18239823,  0.06462966,  0.222618  ,
   -0.61394108,  0.39476297, -0.40503639,  0.15853003,  0.19420555,
    0.05533674, -0.27990161,  0.30253379,  0.01823931,  0.08015055,
   -0.12955907, -0.14429495, -0.15414152,  0.21072053, -0.57952318,
   -0.74220145,  0.54043792, -0.18613938,  0.77424217, -0.65388111,
    0.11644648, -0.46681308
  };
  std::vector<double> penGradY =
  {
    -0.0930594 , -0.0556677 , -0.02202051,  0.15261682,  0.25090717,
    -0.06142548,  0.39829304,  0.55748205, -0.10083045, -0.20612032,
    -0.52324638, -0.81835981,  0.26115301, -0.29670936, -0.07335958,
     0.72842634, -0.07017234,  0.67164066,  0.12359707, -0.12986258,
     0.6980838 , -0.17114781,  0.05127547,  0.06433461, -0.46589269,
     0.23241353, -0.56941459
  };
  std::vector<double> penGradZ =
  {
    -0.10654716,  0.28446888,  0.67548491, -0.06915546,  0.31998833,
     0.70913211,  0.09003355,  0.07042658,  0.05081961,  0.40595893,
     0.2868227 ,  0.16768647,  0.08883288, -0.01929704, -0.12742697,
    -0.70976581, -0.04768988,  0.61438604, -0.38428478, -0.18963829,
     0.00500821, -0.63774443,  0.09760508,  0.83295459,  0.06056178,
     0.04584413,  0.03112647
  };

  std::vector<Vector3d> points =
  {
    {0.23352768013842229, 1.7767243859557733, 0.26601814042029426},
    {2.2485555785893334, 0.55614005239213393, 1.269660653465275},
    {2.1185875346180856, 2.071751514751571, 1.509121136794852},
    {1.8537692322878447, 1.2411053144925532, 1.4575584431698618},
    {1.3542112341981949, 0.85807431414450286, 2.6060901323821746}
  };

  std::vector<double> penRes =
  {
    0.46275887,  0.        ,  0.        ,  0.39354775,  0.
  };

  std::vector<Vector3d> penGradRes =
  {
    {0.11422513837340077, 0.39239411239478911, -0.037287388077032885},
    {0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0},
    {0.13306870948137786, -0.0026793299369999649, 0.29066137572319539},
    {0.0, 0.0, 0.0}
  };


  tpg::ObsPen obsp;
  obsp.setPen(Vector3d::Zero(), Vector3d(1., 1., 1.), 3, 3, 3,
              pen, penGradX, penGradY, penGradZ);

  for(std::size_t i = 0; i < points.size(); ++i)
  {
    BOOST_CHECK_SMALL(obsp.penality(points[i]) - penRes[i], 1e-4);
    BOOST_CHECK_SMALL((obsp.penalityGrad(points[i]) - penGradRes[i]).norm(), 1e-4);
  }
}

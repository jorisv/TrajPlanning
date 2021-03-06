// This file is part of TrajPlanning.
//
// TrajPlanning is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrajPlanning is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TrajPlanning.  If not, see <http://www.gnu.org/licenses/>.

// associated header
#include "ObsPen.h"

// includes
// SCD
#include <SCD/S_Object/S_Sphere.h>
#include <SCD/CD/CD_Pair.h>

namespace tpg
{


Eigen::Vector3d pointToArray(const Eigen::Vector3d& point,
                             const Eigen::Vector3d& start,
                             const Eigen::Array3d& scale)
{
  return (point - start).array()/scale;
}


bool inBound(double val, std::size_t size)
{
  return val > 0. && val < (size - 1);
}


template<typename T>
struct zero_value
{
};

template<>
struct zero_value<double>
{
  static double zero() { return 0.; }
};

template<>
struct zero_value<Eigen::Vector3d>
{
  static Eigen::Vector3d zero() { return Eigen::Vector3d::Zero(); }
};


template<typename T>
T interpolate3d(const boost::multi_array<T, 3>& array,
                const Eigen::Vector3d& arrPoint)
{
  if(!inBound(arrPoint.x(), array.shape()[0]) ||
     !inBound(arrPoint.y(), array.shape()[1]) ||
     !inBound(arrPoint.z(), array.shape()[2]))
  {
    return zero_value<T>::zero();
  }

  int x0 = int(std::floor(arrPoint.x()));
  int x1 = int(std::floor(arrPoint.x() + 1.));
  int y0 = int(std::floor(arrPoint.y()));
  int y1 = int(std::floor(arrPoint.y() + 1.));
  int z0 = int(std::floor(arrPoint.z()));
  int z1 = int(std::floor(arrPoint.z() + 1.));

  double xd = arrPoint.x() - x0;
  double xdm = x1 - arrPoint.x();
  double yd = arrPoint.y() - y0;
  double ydm = y1 - arrPoint.y();
  double zd = arrPoint.z() - z0;
  double zdm = z1 - arrPoint.z();

  T c00 = xdm*array[x0][y0][z0] + xd*array[x1][y0][z0];
  T c10 = xdm*array[x0][y1][z0] + xd*array[x1][y1][z0];
  T c01 = xdm*array[x0][y0][z1] + xd*array[x1][y0][z1];
  T c11 = xdm*array[x0][y1][z1] + xd*array[x1][y1][z1];

  T c0 = ydm*c00 + yd*c10;
  T c1 = ydm*c01 + yd*c11;

  return zdm*c0 + zd*c1;
}


ObsPen& ObsPen::operator=(const ObsPen& op)
{
  pen_.resize(boost::extents[op.pen_.shape()[0]][op.pen_.shape()[1]][op.pen_.shape()[2]]);
  pen_ = op.pen_;
  penGrad_.resize(boost::extents[op.penGrad_.shape()[0]][op.penGrad_.shape()[1]][op.penGrad_.shape()[2]]);
  penGrad_ = op.penGrad_;
  start_ = op.start_;
  scale_ = op.scale_;
  return *this;
}


void ObsPen::setPen(const Eigen::Vector3d& start, const Eigen::Vector3d& scale,
    int sizeX, int sizeY, int sizeZ,
    const std::vector<double>& penality,
    const std::vector<double>& penalityGradX,
    const std::vector<double>& penalityGradY,
    const std::vector<double>& penalityGradZ)
{
  start_ = start;
  scale_ = scale;
  pen_.resize(boost::extents[sizeX][sizeY][sizeZ]);
  penGrad_.resize(boost::extents[sizeX][sizeY][sizeZ]);

  std::memcpy(pen_.data(), penality.data(), sizeX*sizeY*sizeZ*sizeof(double));

  int i = 0;
  for(int x = 0; x < sizeX; ++x)
  {
    for(int y = 0; y < sizeY; ++y)
    {
      for(int z = 0; z < sizeZ; ++z)
      {
        penGrad_[x][y][z] = Eigen::Vector3d(penalityGradX[i],
                                            penalityGradY[i],
                                            penalityGradZ[i]);
        ++i;
      }
    }
  }
}


double ObsPen::penality(const Eigen::Vector3d& pos) const
{
  return interpolate3d(pen_, pointToArray(pos, start_, scale_));
}


Eigen::Vector3d ObsPen::penalityGrad(const Eigen::Vector3d& pos) const
{
  return interpolate3d(penGrad_, pointToArray(pos, start_, scale_));
}

} // tpg

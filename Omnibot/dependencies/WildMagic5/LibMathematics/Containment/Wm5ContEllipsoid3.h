// Geometric Tools, LLC
// Copyright (c) 1998-2012
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.1 (2010/10/01)

#ifndef WM5CONTELLIPSOID3_H
#define WM5CONTELLIPSOID3_H

#include "Wm5MathematicsLIB.h"
#include "Wm5Ellipsoid3.h"
#include "Wm5Line3.h"

namespace Wm5
{

// The input points are fit with a Gaussian distribution.  The center C of the
// ellipsoid is chosen to be the mean of the distribution.  The axes of the
// ellipsoid are chosen to be the eigenvectors of the covariance matrix M.
// The shape of the ellipsoid is determined by the absolute values of the
// eigenvalues.
//
// WARNING.  The construction is ill-conditioned if the points are (nearly)
// collinear or (nearly) planar.  In this case M has a (nearly) zero
// eigenvalue, so inverting M is problematic.
template <typename Real> WM5_MATHEMATICS_ITEM
Ellipsoid3<Real> ContEllipsoid (int numPoints, const Vector3<Real>* points);

// Project an ellipsoid onto a line.  The projection interval is [smin,smax]
// and corresponds to the line segment P+s*D, where smin <= s <= smax.
template <typename Real> WM5_MATHEMATICS_ITEM
void ProjectEllipsoid (const Ellipsoid3<Real>& ellipsoid,
   const Line3<Real>& line, Real& smin, Real& smax);

// Construct a bounding ellipsoid for the two input ellipsoids.
template <typename Real> WM5_MATHEMATICS_ITEM
const Ellipsoid3<Real> MergeEllipsoids (const Ellipsoid3<Real>& ellipsoid0,
    const Ellipsoid3<Real>& ellipsoid1);

}

#endif

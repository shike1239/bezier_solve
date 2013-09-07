/*
  
  Copyright 2013 Lucas Walter

     This file is part of bezier_solve.

    Vimjay is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vimjay is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vimjay.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "opencv2/highgui/highgui.hpp"

// bash color codes
#define CLNRM "\e[0m"
#define CLWRN "\e[0;43m"
#define CLERR "\e[1;41m"
#define CLVAL "\e[1;36m"
#define CLTXT "\e[1;35m"
// BOLD black text with blue background
#define CLTX2 "\e[1;44m"  


// from vimjay
  bool getBezier(
      const std::vector<cv::Point2f>& control_points, // TBD currently has to be 4
      std::vector<cv::Point2f>& output_points,
      const int num // number of intermediate points to generate 
      )
  {
    if (control_points.size() != 4) {
      LOG(ERROR) << control_points.size() << " != 4"; 
      return false;
    }

    /*
    // 2nd order 1 2 1
    double coeff_raw[4][4] = {
      { 1, 0, 0},
      {-2, 2, 0},
      { 1,-2, 1},
    };
    // 4th order 1 4 6 4 1

    general pattern
    bc(1) =    1 1
    bc(2) =   1 2 1
    bc(3) =  1 3 3 1
    bc(4) = 1 4 6 4 1
    
    bc(3,0) = 1
    bc(3,1) = 3

    (1-x)(1-x)(1-x) = 1 -3x 3x^2 -x^3
    (1 -2x x^2) (1-x) 
    
    bc(+/-0) =   1
    bc(-1) =    1 -1
    bc(-2) =   1 -2  1
    bc(-3) =  1 -3  3 -1
    bc(-4) = 1 -4  6 -4  1 
    ...

      { bc(-3)*bc(3,0),  0               0               0
                        bc(-2)*bc(3,1)   0               0
                                         bc(-1)*bc(3,2)  0
                                                         bc(-0)*bc(3,3)

    bc(3,0) is 1, bc(3,1) is 3, etc.
    
    Next higher order desired matrix:

       1  0   0   0  0
      -4  4   0   0  0
       6 -12  6   0  0
      -4  12 -12  4  0
       1 -4   6  -4  1   

    */

    // TBD how to generate programmatically
    // 1 3 3 1
    double coeff_raw[4][4] = {
      { 1, 0, 0, 0},
      {-3, 3, 0, 0},
      { 3,-6, 3, 0},
      {-1, 3,-3, 1}
    };
    cv::Mat coeff = cv::Mat(4, 4, CV_64F, coeff_raw);
    cv::Mat control = cv::Mat::zeros(4, 2, CV_64F);
    
    for (int i = 0; i < control.rows; i++) {
      control.at<double>(i, 0) = control_points[i].x;
      control.at<double>(i, 1) = control_points[i].y;
    }

    VLOG(5) << CLTXT << "coeff " << CLNRM << std::endl << logMat(coeff); 
    VLOG(5) << CLTXT <<"control " << CLNRM << std::endl << logMat(control); 

    cv::Point2f old_pt;

    output_points.clear();

    for (int i = 0; i < num; i++) {
      float t = (float)i/(float)(num-1);

      // concentrate samples near beginning and end
      if (t < 0.5) {
        t *= t;
      } else {
        t = 1.0 - (1.0-t)*(1.0-t);
      }
      double tee_raw[1][4] = {{ 1.0, t, t*t, t*t*t}};

      cv::Mat tee = cv::Mat(1, 4, CV_64F, tee_raw);
      cv::Mat pos = tee * coeff * control;

      cv::Point new_pt = cv::Point2f(pos.at<double>(0,0), pos.at<double>(0,1));

      output_points.push_back(new_pt);

      VLOG(5) << "pos " << t << " "
        << new_pt.x << " " << new_pt.y 
        << std::endl << logMat(tee) 
        << std::endl << logMat(pos); 
    }

    return true;
  }
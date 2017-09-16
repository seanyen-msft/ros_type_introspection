/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright 2016 Davide Faconti
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage, Inc. nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

#ifndef ROS_INTRO_DESERIALIZE_H
#define ROS_INTRO_DESERIALIZE_H

#include <array>
#include <sstream>
#include "ros_type_introspection/parser.hpp"
#include "ros_type_introspection/stringtree.hpp"
#include "ros_type_introspection/variant.hpp"


namespace RosIntrospection{

/**
 * @brief The StringTreeLeaf is, as the name suggests, a leaf (terminal node)
 * of a StringTree.
 * It provides the pointer to the node and a list of numbers that represent
 * the index that corresponds to the placeholder "#".
 *
 * For example if you want to represent the string
 *
 *      foo/2/bar/3/hello/world
 *
 * This would correspond to a branch of the tree (from root to the leaf) equal to these 6 nodes,
 * where "foo" is the root and "world" is the leaf
 *
 * foo -> # -> bar -> # ->hello -> world
 *
 * array_size will be equal to two and index_array will contain these numbers {2,3}
 *
 */
struct StringTreeLeaf{

  StringTreeLeaf();

  StringTreeNode* node_ptr;

  uint8_t array_size;

  std::array<uint16_t,7> index_array;

  /// Utility functions to print the entire branch
  bool toStr(SString &destination) const;
  bool toStr(std::string &destination) const;
  
  // return string length or -1 if failed
  int toStr(char* buffer) const;
  
  SString toSString() { SString out; toStr(out); return out; }
  
  std::string toStdString() { std::string out; toStr(out); return out; }
};

typedef struct{
  /// Tree that the StringTreeLeaf(s) refer to.
  StringTree tree;

  /// List of all those parsed fields that can be represented by a builtin value different from "string".
  /// This list will be filled by the funtion buildRosFlatType.
  std::vector< std::pair<StringTreeLeaf, Variant> > value;

  /// Ñist of all those parsed fields that can be represented by a builtin value equal to "string".
  /// This list will be filled by the funtion buildRosFlatType.
  std::vector< std::pair<StringTreeLeaf, SString> > name;

  // Not used yet
  std::vector< std::pair<StringTreeLeaf, std::vector<uint8_t>>> blob;

}ROSTypeFlat;


/**
 * @brief buildRosFlatType is a function that read raw serialized data from a ROS message (generated by
 * a ros bag or a topic) and stored the values of each field in a "flat" container called ROSTypeFlat.
 * For example if you apply this to [geometry_msgs/Pose](http://docs.ros.org/kinetic/api/geometry_msgs/html/msg/Pose.html)
 * the vector ROSTypeFlat::value will contain the following pairs (where ... is the number of that field) :
 *
 *  - Pose.Point.x = ...
 *  - Pose.Point.y = ...
 *  - Pose.Point.z = ...
 *  - Pose.Quaternion.x = ...
 *  - Pose.Quaternion.y = ...
 *  - Pose.Quaternion.z = ...
 *  - Pose.Quaternion.w = ...
 *
 * IMPORTANT: this approach is not meant to be used with use arrays such as maps, point clouds and images.
 * It would require a ridicoulous amount of memory and, franckly, make little sense.
 * For this reason the argument max_array_size is used.
 *
 * @param type_map               List of all the ROSMessage already known by the application (built using buildROSTypeMapFromDefinition)
 * @param type                   The main type that correspond to this serialized data.
 * @param prefix                 Prefix to add to the name (actually, the root of StringTree).
 * @param buffer                 The raw buffer to be parsed
 * @param flat_container_output  It is recommended to reuse the same object if possible to reduce the amount of memory allocation.
 * @param max_array_size         All the vectors that contains more elements than max_array_size will be discarted.
 */

void buildRosFlatType(const ROSTypeList& type_map,
                      ROSType type,
                      SString prefix,
                      const nonstd::VectorView<uint8_t>& buffer,
                      ROSTypeFlat* flat_container_output,
                      const uint32_t max_array_size );

inline std::ostream& operator<<(std::ostream &os, const StringTreeLeaf& leaf )
{
  SString dest;
  leaf.toStr(dest);
  os << dest;
  return os;
}


//-------------------- UTILITY function ------------------
// Brutally faster for numbers below 100
inline int print_number(char* buffer, uint16_t value)
{
    const char DIGITS[] =
            "00010203040506070809"
            "10111213141516171819"
            "20212223242526272829"
            "30313233343536373839"
            "40414243444546474849"
            "50515253545556575859"
            "60616263646566676869"
            "70717273747576777879"
            "80818283848586878889"
            "90919293949596979899";
    if (value < 10)
    {
        buffer[0] = static_cast<char>('0' + value);
        return 1;
    }
    else if (value < 100) {
        value *= 2;
        buffer[0] = DIGITS[ value+1 ];
        buffer[1] = DIGITS[ value ];
        return 2;
    }
    else{
        return sprintf( buffer,"%d", value );
    }
}


} //end namespace

#endif // ROS_INTRO_DESERIALIZE_H

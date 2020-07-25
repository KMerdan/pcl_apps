// Copyright (c) 2019 OUXT Polaris
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <pcl_apps/filter/voxelgrid_filter/voxelgrid_filter_component.hpp>

// Headers in ROS2
#include <rclcpp_components/register_node_macro.hpp>

// Headers in STL
#include <string>
#include <vector>
#include <memory>

namespace pcl_apps
{
VoxelgridFilterComponent::VoxelgridFilterComponent(const rclcpp::NodeOptions & options)
: Node("voxelgrid_filter", options)
{
  declare_parameter("leaf_size", 1.0);
  get_parameter("leaf_size", leaf_size_);
  declare_parameter("input_topic", get_name() + std::string("/input"));
  get_parameter("input_topic", input_topic_);
  set_on_parameters_set_callback(
    [this](const std::vector<rclcpp::Parameter> params) -> rcl_interfaces::msg::SetParametersResult
    {
      auto results = std::make_shared<rcl_interfaces::msg::SetParametersResult>();
      for (auto param : params) {
        if (param.get_name() == "leaf_size") {
          if (leaf_size_ > 0) {
            leaf_size_ = param.as_double();
            results->successful = true;
            results->reason = "";
          } else {
            results->successful = false;
            results->reason = "leaf size must over 0";
          }
        }
      }
      if (!results->successful) {
        results->successful = false;
        results->reason = "";
      }
      return *results;
    }
  );
  std::string output_topic_name = get_name() + std::string("/output");
  pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(output_topic_name, 10);
  auto callback =
    [this](const typename sensor_msgs::msg::PointCloud2::SharedPtr msg) -> void
    {
      pcl::PCLPointCloud2::Ptr cloud(new pcl::PCLPointCloud2());
      pcl_conversions::toPCL(*msg, *cloud);
      filter_.setInputCloud(cloud);
      assert(leaf_size_ > 0.0);
      filter_.setLeafSize(leaf_size_, leaf_size_, leaf_size_);
      pcl::PCLPointCloud2::Ptr cloud_filtered(new pcl::PCLPointCloud2());
      filter_.filter(*cloud_filtered);
      sensor_msgs::msg::PointCloud2 output_cloud_msg;
      pcl_conversions::fromPCL(*cloud_filtered, output_cloud_msg);
      pub_->publish(output_cloud_msg);
    };
  sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(input_topic_, 10, callback);
}
}  // namespace pcl_apps

RCLCPP_COMPONENTS_REGISTER_NODE(pcl_apps::VoxelgridFilterComponent)

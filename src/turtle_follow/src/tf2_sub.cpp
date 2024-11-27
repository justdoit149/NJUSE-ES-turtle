/*
    订阅 turtle1 和 turtle2 的 TF 广播信息，查找并转换时间最近的 TF 信息
    将 turtle1 转换成相对 turtle2 的坐标，在计算线速度和角速度并发布
*/
// 1.包含头文件
#include "ros/ros.h"
#include "tf2_ros/transform_listener.h"
#include "geometry_msgs/TransformStamped.h"
#include "geometry_msgs/Twist.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    // 2.初始化 ros 节点
    ros::init(argc, argv, "tf2_sub");
    // 3.创建 ros 句柄
    ros::NodeHandle nh;
    // 4.创建 TF 订阅对象
    tf2_ros::Buffer buffer;
    tf2_ros::TransformListener listener(buffer);
    // 5.处理订阅到的 TF
    // 需要创建发布 /turtle2/cmd_vel 的 publisher 对象
    ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("/turtle2/cmd_vel", 1000);
    ros::Rate rate(10);
    // 定义两个距离阈值
    const double k1 = 3.0; // 全速跟随阈值
    const double k2 = 1.0; // 等待阈值
    while (ros::ok())
    {
        try
        {
            // 5-1.先获取 turtle1 相对 turtle2 的坐标信息
            geometry_msgs::TransformStamped tfs = buffer.lookupTransform("turtle2", "turtle1", ros::Time(0));
            // 5-2.根据坐标信息生成速度信息 -- geometry_msgs/Twist.h
            geometry_msgs::Twist twist;
            // 计算两者之间的距离
            double distance = sqrt(pow(tfs.transform.translation.x, 2) + pow(tfs.transform.translation.y, 2));
            if (distance > k1)
            {
                twist.linear.x = 2.0; // 全速
            }
            else if (distance > k2)
            {
                twist.linear.x = 1.0; // 降速
            }
            else
            {
                twist.linear.x = 0.0; // 停止
            }
            // twist.linear.x = 0.5 * sqrt(pow(tfs.transform.translation.x, 2) + pow(tfs.transform.translation.y, 2));
            twist.angular.z = 4 * atan2(tfs.transform.translation.y, tfs.transform.translation.x);
            // 5-3.发布速度信息 -- 需要提前创建 publish 对象
            pub.publish(twist);
        }
        catch (const std::exception &e)
        {
            // std::cerr << e.what() << '\n';
            ROS_INFO("错误提示:%s", e.what());
        }
        rate.sleep();
        // 6.spin
        ros::spinOnce();
    }
    return 0;
}

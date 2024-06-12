// created by lz 2024-06-12
#include <pthread.h>
#include <string.h> // memcpy
#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <linux/joystick.h>  
#include <iostream>
#include <signal.h>
#include <mutex>

#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <sensor_msgs/JointState.h>
#define XBOX_NUMCONSTANT    (32767.0f)
#define XBOX_TYPE_BUTTON    0x01  
#define XBOX_TYPE_AXIS      0x02  

#define XBOX_BUTTON_A       0x00  
#define XBOX_BUTTON_B       0x01  
#define XBOX_BUTTON_X       0x02  
#define XBOX_BUTTON_Y       0x03  
#define XBOX_BUTTON_LB      0x04  
#define XBOX_BUTTON_RB      0x05  
// MOCUTE
// #define XBOX_BUTTON_START   0x06  
// #define XBOX_BUTTON_BACK    0x07  
#define XBOX_BUTTON_START   0x07  
#define XBOX_BUTTON_SELECT  0x06
#define XBOX_BUTTON_LO      0x08    // 左侧控制下压
#define XBOX_BUTTON_RO      0x09    //右侧下压

#define XBOX_BUTTON_ON      0x01  
#define XBOX_BUTTON_OFF     0x00  
//      /\ y
// x    |
// <-----
// 
#define XBOX_AXIS_LX        0x00    /* 左摇杆X轴 */  
#define XBOX_AXIS_LY        0x01    /* 左摇杆Y轴 */  
#define XBOX_AXIS_RX        0x03    /* 右摇杆X轴 */  
#define XBOX_AXIS_RY        0x04    /* 右摇杆Y轴 */  
#define XBOX_AXIS_LT        0x02  
#define XBOX_AXIS_RT        0x05  
#define XBOX_AXIS_XX        0x06    /* 方向键X轴 */  
#define XBOX_AXIS_YY        0x07    /* 方向键Y轴 */  

#define XBOX_AXIS_VAL_UP        -32767  
#define XBOX_AXIS_VAL_DOWN      32767  
#define XBOX_AXIS_VAL_LEFT      -32767  
#define XBOX_AXIS_VAL_RIGHT     32767  

#define XBOX_AXIS_VAL_MIN       -32767  
#define XBOX_AXIS_VAL_MAX       32767  
#define XBOX_AXIS_VAL_MID       0x00  
typedef struct xbox_map  
{  
    int     time;  
    int     a;  
    int     b;  
    int     x;  
    int     y;  
    int     lb;  
    int     rb;  
    int     start;  
    int     back;  
    int     select;
    int     home;  
    int     lo;  
    int     ro;  
    // aixes
    int     lx;  
    int     ly;  
    int     rx;  
    int     ry;  
    int     lt;  
    int     rt;  
    int     xx;  
    int     yy;  

}xbox_map_t;  

int xbox_open(const char *file_name)  
{  
    int xbox_fd;  

    xbox_fd = open(file_name, O_RDONLY);  
    if (xbox_fd < 0)  
    {  
        perror("open");  
        return -1;  
    }  

    return xbox_fd;  
}  

int xbox_map_read(int xbox_fd, xbox_map_t *map)  
{  
    int len, type, number, value;  
    struct js_event js;  

    len = read(xbox_fd, &js, sizeof(struct js_event));  
    if (len < 0)  
    {  
        perror("read");  
        return -1;  
    }  

    type = js.type;  
    number = js.number;  
    value = js.value;  

    map->time = js.time;  

    if (type == JS_EVENT_BUTTON)  
    {  
        switch (number)  
        {  
            case XBOX_BUTTON_A:  
                map->a = value;  
                break;  

            case XBOX_BUTTON_B:  
                map->b = value;  
                break;  

            case XBOX_BUTTON_X:  
                map->x = value;  
                break;  

            case XBOX_BUTTON_Y:  
                map->y = value;  
                break;  

            case XBOX_BUTTON_LB:  
                map->lb = value;  
                break;  

            case XBOX_BUTTON_RB:  
                map->rb = value;  
                break;  

            case XBOX_BUTTON_START:  
                map->start = value;  
                break;  

            case XBOX_BUTTON_SELECT:  
                map->select = value;  
                break;  

            // case XBOX_BUTTON_HOME:  //no
            //     map->home = value;  
            //     break;  

            case XBOX_BUTTON_LO:  //logitech button
                map->lo = value;  
                break;  

            case XBOX_BUTTON_RO:   //no
                map->ro = value;  
                break;  

            default:  
                break;  
        }  
    }  
    else if (type == JS_EVENT_AXIS)  
    {  
        switch(number)  
        {  
            case XBOX_AXIS_LX:  
                map->lx = value;  
                break;  

            case XBOX_AXIS_LY:  
                map->ly = value;  
                break;  

            case XBOX_AXIS_RX:  
                map->rx = value;  
                break;  

            case XBOX_AXIS_RY:  
                map->ry = value;  
                break;  

            case XBOX_AXIS_LT:  
                map->lt = value;  
                break;  

            case XBOX_AXIS_RT:  
                map->rt = value;  
                break;  

            case XBOX_AXIS_XX:  //方向键
                map->xx = value;  
                break;  

            case XBOX_AXIS_YY:  
                map->yy = value;  
                break;  

            default:  
                break;  
        }  
    }  
    else  
    {  
        /* Init do nothing */  
    }  

    return len;  
}  

void xbox_close(int xbox_fd)  
{  
    close(xbox_fd);  
    return;  
}  

sensor_msgs::Joy convertToJoyMsg(const xbox_map_t& xbox_data) {
    sensor_msgs::Joy joy_msg;

    // Fill in the axes
    joy_msg.axes.resize(8);
    joy_msg.axes[0] = xbox_data.lx/XBOX_NUMCONSTANT; //左边的摇杆
    joy_msg.axes[1] = xbox_data.ly/XBOX_NUMCONSTANT;
    joy_msg.axes[2] = xbox_data.rx/XBOX_NUMCONSTANT; // 右边的摇杆
    joy_msg.axes[3] = xbox_data.ry/XBOX_NUMCONSTANT;
    joy_msg.axes[4] = xbox_data.lt/XBOX_NUMCONSTANT;
    joy_msg.axes[5] = xbox_data.rt/XBOX_NUMCONSTANT;
    joy_msg.axes[6] = xbox_data.xx/XBOX_NUMCONSTANT;
    joy_msg.axes[7] = xbox_data.yy/XBOX_NUMCONSTANT;
    // Fill in the buttons
    joy_msg.buttons.resize(10);
    joy_msg.buttons[0] = xbox_data.a;
    joy_msg.buttons[1] = xbox_data.b;
    joy_msg.buttons[2] = xbox_data.x;
    joy_msg.buttons[3] = xbox_data.y;
    joy_msg.buttons[4] = xbox_data.lb;
    joy_msg.buttons[5] = xbox_data.rb;
    joy_msg.buttons[6] = xbox_data.start;
    joy_msg.buttons[7] = xbox_data.back;
    joy_msg.buttons[8] = xbox_data.select;
    joy_msg.buttons[9] = xbox_data.lo;

    // Additional values (xx and yy) could be included as additional axes or buttons if needed

    return joy_msg;
}
sensor_msgs::JointState convertToJointStateMsg(const xbox_map_t& xbox_data) {
    sensor_msgs::JointState joint_state_msg;

    // Define joint names corresponding to xbox_map_t members
    std::vector<std::string> joint_names = {
        "button_a", "button_b", "button_x", "button_y",
        "button_lb", "button_rb", "button_start", "button_back",
        "button_select", "button_home", "button_lo", "button_ro",
        "stick_lx", "stick_ly", "stick_rx", "stick_ry",
        "trigger_lt", "trigger_rt", "dpad_xx", "dpad_yy"
    };

    // Fill joint names
    joint_state_msg.name = joint_names;

    // Fill joint positions
    joint_state_msg.position = {
        static_cast<double>(xbox_data.a),                     // 0                
        static_cast<double>(xbox_data.b),                     // 1           
        static_cast<double>(xbox_data.x),                     // 2           
        static_cast<double>(xbox_data.y),                     // 3           
        static_cast<double>(xbox_data.lb),                    // 4          
        static_cast<double>(xbox_data.rb),                    // 5           
        static_cast<double>(xbox_data.start),                 // 6           
        static_cast<double>(xbox_data.back),                  // 7          
        static_cast<double>(xbox_data.select),                // 8          
        static_cast<double>(xbox_data.home),                  // 9        
        static_cast<double>(xbox_data.lo),                    // 10   
        static_cast<double>(xbox_data.ro),                    // 11   
        static_cast<double>(xbox_data.lx/XBOX_NUMCONSTANT),   // 12   
        static_cast<double>(xbox_data.ly/XBOX_NUMCONSTANT),   // 13   
        static_cast<double>(xbox_data.rx/XBOX_NUMCONSTANT),   // 14   
        static_cast<double>(xbox_data.ry/XBOX_NUMCONSTANT),   // 15   
        static_cast<double>(xbox_data.lt/XBOX_NUMCONSTANT),   // 16   
        static_cast<double>(xbox_data.rt/XBOX_NUMCONSTANT),   // 17   
        static_cast<double>(xbox_data.xx/XBOX_NUMCONSTANT),   // 18   
        static_cast<double>(xbox_data.yy/XBOX_NUMCONSTANT)    // 19  
    };

    return joint_state_msg;
}

void sigintHandler(int sig) {
    ros::shutdown();
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "joy_pub",ros::init_options::NoSigintHandler);
    ros::NodeHandle n;
    ros::Publisher joy_pub = n.advertise<sensor_msgs::JointState>("joy_status", 1000);
    ros::Rate loop_rate(100);
    xbox_map_t map_data;
    std::mutex mutex_map_data;
    sensor_msgs::Joy joy_pub_msg;
    int port=0;
    try
    {
        port = xbox_open("/dev/input/js0");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    signal(SIGINT, sigintHandler);
    memset(&map_data, 0, sizeof(xbox_map_t));
    while (ros::ok()){

        int len = xbox_map_read(port, &map_data); 

        // printf("joy a:%d, b:%d lo:%d x:%d y:%d xx:%d yy:%d\n",map_data.a, map_data.b, map_data.lo,map_data.x,map_data.y,map_data.xx,map_data.yy);
        // printf("joy  lt=%d lx=%d  ly=%d  rt=%d rx=%d  ry=%d start=%d select:%d\n",map_data.lt,map_data.lx,map_data.ly,map_data.rt,map_data.rx,map_data.ry,map_data.start,map_data.select);    

        // sensor_msgs::Joy joy_msg = convertToJoyMsg(map_data);
         sensor_msgs::JointState joy_msg = convertToJointStateMsg(map_data);

        joy_msg.header.stamp = ros::Time::now();

        joy_pub.publish(joy_msg);

        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}
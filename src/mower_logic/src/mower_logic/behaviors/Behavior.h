// Created by Clemens Elflein on 2/21/22.
// Copyright (c) 2022 Clemens Elflein. All rights reserved.
//
// This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// Feel free to use the design in your private/educational projects, but don't try to sell the design or products based on it without getting my consent first.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
#ifndef SRC_BEHAVIOR_H
#define SRC_BEHAVIOR_H

#include "ros/ros.h"
#include "mower_logic/MowerLogicConfig.h"

/**
 * Behavior definition
 */
class Behavior {

private:
    ros::Time startTime;
    ros::Time lastOdomTime;
    long valid_odom_counter;

protected:
    bool aborted;
    bool paused;

    bool requested_continue_flag;
    bool requested_pause_flag;

    double time_in_state() {
        return (ros::Time::now() - startTime).toSec();
    }

    mower_logic::MowerLogicConfig config;

    /**
     * Called ONCE on state enter.
     */
    virtual void enter() = 0;

public:

    virtual std::string state_name() = 0;

    void updateOdomTime(void)
    {
        lastOdomTime = ros::Time::now();
    }

    // /odom is valid for 5 seconds
    // (assumes being called once a second)
    bool isOdomValid()
    {
        if ( (ros::Time::now() - lastOdomTime).toSec() < 1.0 ) // /odom received within last second
        {
            valid_odom_counter++;
        }
        else
        {
            valid_odom_counter = 0; // reset
        }
        return ( valid_odom_counter > 5.0 );
    } 

    void requestContinue()
    {
        requested_continue_flag = true;
    }

    void requestPause()
    {
        requested_pause_flag = true;
    }

    void setPause()
    {
        paused = true;
    }

    void setContinue()
    {
        paused = false;
        requested_continue_flag = false;
        requested_pause_flag = false;
    }

    void start(mower_logic::MowerLogicConfig &c) {
        ROS_INFO_STREAM("");
        ROS_INFO_STREAM("");
        ROS_INFO_STREAM("--------------------------------------");
        ROS_INFO_STREAM("- Entered state: " << state_name());
        ROS_INFO_STREAM("--------------------------------------");
        aborted = false;
        paused = false;
        requested_continue_flag = false;
        requested_pause_flag = false;
        this->config = c;
        startTime = ros::Time::now();
        valid_odom_counter = 0;
        enter();
    }

    /**
     * Execute the behavior. This call should block until the behavior is executed fully.
     * @returns the pointer to the next behavior (can return itself).
     */
    virtual Behavior *execute() = 0;

    /**
     * Called ONCE before state exits
     */
    virtual void exit() = 0;

    /**
     * Reset the internal state of the behavior.
     */
    virtual void reset() = 0;

    /**
     * If called, save state internally and return the execute() method asap.
     * Execution should resume on the next execute() call.
     */
    void abort() {
        ROS_INFO_STREAM("- Behaviour.h: abort() called");
        aborted = true;
    }

    // Return true, if this state needs absolute positioning.
    // The state will be aborted if GPS is lost and resumed at some later point in time.
    virtual bool needs_gps() = 0;

    // return true, if the mower motor should currently be running.
    virtual bool mower_enabled() = 0;

    // return true to redirect joystick speeds to the controller
    virtual bool redirect_joystick() = 0;


    virtual void command_home() = 0;
    virtual void command_start() = 0;
    virtual void command_s1() = 0;
    virtual void command_s2() = 0;
};

#endif //SRC_BEHAVIOR_H

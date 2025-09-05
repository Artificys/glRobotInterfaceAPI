#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include "interface/interface.h"

std::atomic<bool> stopFlag { false };

auto signalHandler = [](int) {
    stopFlag = true;
};

/**
 *  Example of how to use the Interface class to load a URDF model and control its joints.
 */
int main()
{
    std::signal(SIGINT, signalHandler);

    // Example joint values (shared with Interface)
    std::atomic<float> canvasJointEncoder { 0.0f };
    std::atomic<float> baseHubEncoder { 0.0f };
    std::atomic<float> armJoint1Encoder { 0.0f };

    Interface interface;
    if (!interface.init("urdf/Robot.urdf")) {
        std::cerr << "Failed to initialize interface.\n";
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for init

    interface.linkJoint("base_to_canvas", canvasJointEncoder);
    interface.linkJoint("base_to_hub_3", baseHubEncoder);
    interface.linkJoint("hub_to_arm_3", armJoint1Encoder);

    interface.spin();

    // Wait until SIGINT is received (And can also raise the signal in child thread)
    for (int i = 0; !stopFlag; i++) {
        canvasJointEncoder = i * 0.01f; // Continuous increasing value for rotation
        baseHubEncoder = 0.5f * sin(i * 0.02f); // Oscillating value for rotation
        armJoint1Encoder = 0.5f * cos(i * 0.03f); // Oscillating value for in/out movement

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    return 0;
}

#include <chrono>
#include <iostream>
#include <thread>
#include <opencv2/imgcodecs.hpp>
#include "CameraScanner.h"

int main()
{
    CameraScanner scanner;

    OperationResult s = scanner.start(0);
    std::cout << "start: " << s.message << "\n";
    if (!s.success) return 1;

    const auto begin = std::chrono::steady_clock::now();
    cv::Mat frame, lastFrame;
    std::string lastMessage;
    bool found = false;

    while (std::chrono::steady_clock::now() - begin < std::chrono::seconds(15)) {
        OperationResult r = scanner.scanNextFrame(frame);
        if (!frame.empty()) lastFrame = frame;

        if (r.message != lastMessage) {          // print only changes
            std::cout << "status: " << r.message << "\n";
            lastMessage = r.message;
        }
        if (r.success) {
            std::cout << "RESULT: " << r.value << "\n";
            found = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    scanner.stop();
    std::cout << "camera released: " << (scanner.isRunning() ? "NO" : "yes") << "\n";

    if (!lastFrame.empty()) {
       std::string debugOutputPath = "output/last_frame.png";
               cv::imwrite(debugOutputPath, lastFrame);
        std::cout << "saved " << debugOutputPath << "\n";
    }
    std::cout << (found ? "PASS" : "No valid code scanned in 15 s") << "\n";
    return found ? 0 : 2;
}
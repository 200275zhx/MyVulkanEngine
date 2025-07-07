#include "test/Test.hpp"

namespace mve {
	void test_taskflow() {
		// ---------- taskflow test ---------- //

		tf::Taskflow taskflow;
		std::vector<int> items{ 1, 2, 3, 4, 5, 6, 7, 8 };

		tf::Task task = taskflow.for_each_index(0u, uint32_t(items.size()), 1u, [&](int i) {
			std::cout << items[i];
			}).name("for each index");

		taskflow.emplace([]() { std::cout << "\nS - Start\n"; }).name("S").precede(task);
		taskflow.emplace([]() { std::cout << "\nT - End\n"; }).name("T").succeed(task);

		std::ofstream os(PROJECT_ROOT_DIR "cache/taskflow/taskflow.dot");
		taskflow.dump(os);
		// run dot -Tpng taskflow.dot -o output.png at PROJECT_ROOT_DIR/cache/taskflow 
		// to get an image generation of taskflow diagram

		tf::Executor executor;
		executor.run(taskflow).wait();
	}

	//void test_ktx(const char* inputDir, const char* outputDir) {
	//	mve::CompressTextureKTX2(PROJECT_ROOT_DIR inputDir, PROJECT_ROOT_DIR outputDir);
	//}

	void test_window() {
		// ---------- window test ---------- //
		constexpr uint32_t WIDTH = 1280;
		constexpr uint32_t HEIGHT = 800;

		GLFWwindow* window = mve::InitWindow("GLFW window test", WIDTH, HEIGHT);

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
		glfwDestroyWindow(window);
		glfwTerminate();
	}



    //static constexpr uint64_t MaxFramesInFlight = 2;
    //void syncManagerUseCase() {
    //    // --- 1) Initialize your Vulkan device as usual (must have requested timelineSemaphore) ---
    //    mve::Device device{};
    //    // ... your swapchain, render-pass, pipeline setup, etc. ...

    //    // --- 2) Create the SyncManager once, after logical device is up ---
    //    mve::SyncManager syncMgr(device);

    //    // Grab the Vulkan graphics queue handle
    //    VkQueue graphicsQueue = device.getGraphicsQueue();

    //    uint64_t frameValue = 0;   // our timeline counter

    //    //while (!shouldExit()) {
    //        // --- 3) Throttle CPU so we never exceed MaxFramesInFlight --- 
    //        // Wait until the semaphore’s counter >= frameValue - MaxFramesInFlight
    //        uint64_t waitValue = (frameValue >= MaxFramesInFlight)
    //            ? (frameValue - MaxFramesInFlight)
    //            : 0;
    //        syncMgr.hostWait(mve::Domain::Graphics, waitValue);

    //        // --- 4) Record your commands into a VkCommandBuffer ---
    //        //VkCommandBuffer cmd = beginGraphicsCommandBuffer();
    //        //recordYourRenderPass(cmd);
    //        //endCommandBuffer(cmd);

    //        // --- 5) Build timeline-semaphore submit infos by hand ---
    //        // a) Acquire the raw semaphore handle (lock-free)
    //        VkSemaphore timelineSem = syncMgr.getSemaphore(mve::Domain::Graphics);

    //        // b) Increment our counter for this frame
    //        ++frameValue;

    //        // c) Fill out VkSemaphoreSubmitInfo structs
    //        VkSemaphoreSubmitInfo waitInfo{
    //            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    //            nullptr,
    //            timelineSem,
    //            waitValue,
    //            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    //        };
    //        VkSemaphoreSubmitInfo signalInfo{
    //            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    //            nullptr,
    //            timelineSem,
    //            frameValue,
    //            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    //        };

    //        // d) Chain in the timeline info
    //        VkTimelineSemaphoreSubmitInfo timelineInfo{
    //            VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
    //            nullptr,
    //            1,
    //            &waitValue,
    //            1,
    //            &frameValue
    //        };

    //        // e) Build the VkSubmitInfo2
    //        VkSubmitInfo2 submitInfo{};
    //        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    //        submitInfo.pNext = &timelineInfo;
    //        submitInfo.flags = 0;  // no special flags
    //        submitInfo.waitSemaphoreInfoCount = 1;
    //        submitInfo.pWaitSemaphoreInfos = &waitInfo;
    //        submitInfo.commandBufferInfoCount = 1;
    //        submitInfo.pCommandBufferInfos = &cmdBufInfo;
    //        submitInfo.signalSemaphoreInfoCount = 1;
    //        submitInfo.pSignalSemaphoreInfos = &signalInfo;

    //        // f) Submit to the graphics queue
    //        VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

    //        // --- 6) Present your rendered image ---
    //        //presentSwapchainImage();

    //        // (Optionally) query how far the GPU got:
    //        // uint64_t completed = syncMgr.completedValue(mve::Domain::Graphics);
    //    }

    //    // ~SyncManager() will vkDeviceWaitIdle + destroy semaphores
    //}
} // namespace mve
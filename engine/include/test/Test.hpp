#pragma once
#include <iostream>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include "tool/HelpersGLFW.hpp"
#include "tool/HelpersKTX.hpp"

namespace mve {
	void test_taskflow();
	void test_ktx(const char* inputDir, const char* outputDir);
	void test_window();
}
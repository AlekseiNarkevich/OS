#include <gtest/gtest.h>
#include <thread>
#include "average.h"
#include "min_max.h"
#include "data.h"

TEST(AverageTest, CalculatesAverageForPositiveNumbers) {
    Data data{ {1.0, 2.0, 3.0, 4.0, 5.0} };
    std::thread thread(calculateAverage, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(3.0, data.average);
}

TEST(AverageTest, CalculatesAverageForNegativeNumbers) {
    Data data{ {-1.0, -2.0, -3.0, -4.0, -5.0} };
    std::thread thread(calculateAverage, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(-3.0, data.average);
}

TEST(AverageTest, CalculatesAverageForMixedNumbers) {
    Data data{ {-1.0, 2.0, -3.0, 4.0, -5.0} };
    std::thread thread(calculateAverage, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(-0.6, data.average);
}

TEST(AverageTest, HandlesSingleElement) {
    Data data{ {42.0} };
    std::thread thread(calculateAverage, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(42.0, data.average);
}

TEST(MinMaxTest, FindsMinMaxForPositiveNumbers) {
    Data data{ {1.0, 2.0, 3.0, 4.0, 5.0} };
    std::thread thread(findMinMax, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(1.0, data.min);
    EXPECT_DOUBLE_EQ(5.0, data.max);
}

TEST(MinMaxTest, FindsMinMaxForNegativeNumbers) {
    Data data{ {-1.0, -2.0, -3.0, -4.0, -5.0} };
    std::thread thread(findMinMax, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(-5.0, data.min);
    EXPECT_DOUBLE_EQ(-1.0, data.max);
}

TEST(MinMaxTest, FindsMinMaxForMixedNumbers) {
    Data data{ {-1.0, 2.0, -3.0, 4.0, -5.0} };
    std::thread thread(findMinMax, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(-5.0, data.min);
    EXPECT_DOUBLE_EQ(4.0, data.max);
}

TEST(MinMaxTest, HandlesSingleElement) {
    Data data{ {42.0} };
    std::thread thread(findMinMax, &data);
    thread.join();
    EXPECT_DOUBLE_EQ(42.0, data.min);
    EXPECT_DOUBLE_EQ(42.0, data.max);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}